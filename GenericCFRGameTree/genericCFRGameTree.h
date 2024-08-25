#pragma once
#include "pch.h"
#include "framework.h"
#include "cfrGameTreeUtils.h"
#include "ChildNodeUtils.h"
#include "gameHistory.h"

#if _WIN32 || _WIN64
	#if _WIN64
		#define ENVIRONMENT64
	#else
		#define ENVIRONMENT32
#endif
#endif

#if __GNUC__
	#if __x86_64__ || __ppc64__
		#define ENVIRONMENT64
	#else
		#define ENVIRONMENT32
	#endif
#endif

template<typename GameState, typename ChanceNode, typename Action>
class CFRGameTree {

public:

	typedef std::vector<Action*> Strategy;

	typedef History<GameState, ChanceNode, Action> GameHistory;
	typedef HistoryNode<GameState, ChanceNode, Action> GameHistoryNode;

	using GameNodeChildren = ChildrenFromGameNode<GameState, ChanceNode, Action>;
	using ChanceNodeChildren = ChildrenFromChanceNode<GameState, Action>;




	CFRGameTree() {

		mChildrenFromGameFunc = nullptr;
		mChildrenFromChanceFunc = nullptr;
		mStartingChanceNode = ChanceNode();

		mPlayerOneHasAction = nullptr;

		mGameTree = nullptr;
		mSizeAtDepth = nullptr;
		mCumulativeOffsetAtDepth = nullptr;
		mUtilityFunc = nullptr;	
	}

	CFRGameTree(
		GameNodeChildren* ( *childNodesFromGameNode )( GameState, Strategy ),
		ChanceNodeChildren* ( *childNodesFromChanceNode ) ( ChanceNode ),
		bool ( *playerOneHasAction ) ( GameState ),
		ChanceNode startingChanceNode,
		float ( *utilityFromHistory )( GameHistory *history )
	) {
		mChildrenFromGameFunc = childNodesFromGameNode;
		mChildrenFromChanceFunc = childNodesFromChanceNode;
		mPlayerOneHasAction = playerOneHasAction;
		mStartingChanceNode = startingChanceNode;
		mUtilityFunc = utilityFromHistory;

		mSizeAtDepth = nullptr;
		mCumulativeOffsetAtDepth = nullptr;
		mGameTree = nullptr;
	}

	long PreProcessor();
	void ConstructTree();
	void CFR(int, float);
	void PrintGameTree();
	long UpdateNashStrategy(long pNodePos, int numIterations);

private:


	/*INSTANCE MEMBERS*/

	/*Pointer to constructed Game tree*/
	byte* mGameTree;

	/*Pointer to root of Game Tree (Chance Node)*/
	ChanceNode mStartingChanceNode;
	
	/*Pointer to list of size in bytes of each level of Game tree */
	std::vector<long> *mSizeAtDepth;			
	
	/*Pointer to list of cumulative offsets of each level of Game Tree*/
	std::vector<long> *mCumulativeOffsetAtDepth;

	/**
	 * @brief User defined function returns list of Child Nodes from
			  a given GameNode, Common Game State, and Strategy Profile List.
	 */
	GameNodeChildren* (*mChildrenFromGameFunc)( GameState gameState, Strategy strategy);

	/**
	 * @brief User defined function returns 2-tuple of list pointers.
	 *		  1st elem is a list of Child Game Nodes with probability from the Chance Node.
	 *		  2nd elem is a list of Child Terminal Nodes with probability from the Chance Node.
	 */
	ChanceNodeChildren* (*mChildrenFromChanceFunc) ( ChanceNode );

	/**
	 * @brief User defined function that gets player who has action for a given gamestate
	 * @param pCurrGameState 
	 * @return true if player 1 has action, false if player 2 has action.
	 */
	bool (* mPlayerOneHasAction) (GameState currGameState);
	
	/**
	 * @brief User defined function that gets utility value from given Terminal Node.
	 * @param pCurrTerminalNode
	 * @return (float) value of utility of pCurrTerminalNode.
	 */
	float ( *mUtilityFunc ) ( GameHistory *history );

	/**
	 * @brief Overloaded Helper functions for pre - processing the game tree.
	 * @param Node*
	 * @return Returns the (long) size of a serialized node and all of its descendants.
	 */
	long PreProcessorHelper(GameState gameState, Strategy strategy, int depth, int uniqueHistoriesCnt);
	long PreProcessorHelperTerminal(int depth, int uniqueHistoriesCnt);
	long PreProcessorHelper(ChanceNode chanceNode, int depth, int uniqueHistoriesCnt);

	/**
	 * @brief Overloaded Helper functions for constructing the game tree
	 *		  Sets bytes of allocated gameTree to appropiate representation.
	 * @param node Pointer to node to be added to the Game Tree
	 * @param depth Current Depth of tree
	 * @param history Pointer to Ordered List of all ancestor nodes and actions.
	 */
	void TreeConstructorHelper(GameState gameState, Strategy strategy, int depth, GameHistory* pHistory, bool setNode);
	void TreeConstructorHelperTerminal(int depth, GameHistory* pHistory);
	void TreeConstructorHelper(ChanceNode chanceNode, int depth, GameHistory* pHistory, bool setNode);
	

	std::pair<float, long>CFRHelper(long pNodePos, float, float);
	std::pair<float, long>CFRGameNodeHelper(TreeGameNode *treeGameNode, float reachProbPlayerOne, float reachProbPlayerTwo);
	std::pair<float, long>CFRChanceNodeHelper(TreeChanceNode *treeChanceNode, float reachProbPlayerOne, float reachProbPlayerTwo);
	
	long UpdateStratProbsRecursive(long pNodePos);
	long UpdateStratGameNode(TreeGameNode *treeGameNode);
	long UpdateStratChanceNode(TreeChanceNode *treeChanceNode);
	
	
	
	/**
	 * @brief Helper function for printing out strategy for each Game Node in
	 *	      the game Tree.
	 * @param pNodePos 
	 * @return (long) offset of next node to evaluate.
	 */
	long PrintTreeHelper(long pNodePos);
	
};


/**
* @brief A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of the required information from a GameNode and all of its descendants in bytes.
*/
template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>
::PreProcessorHelper(GameState gameState, Strategy strategy, int depth, int uniqueHistoriesCnt) {
	
	//Ensure sizeAtDepth is large enough to store value at current depth
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(0);
	}
		
	/*
	Strategy size = number of actions * sizeof(float) * 3 
	Multiplying by 3 is the result of needing to store:
		- Current Strategy
		- Cumulative sum of strategies
		- Immediate counterfactual regret for each action.
	*/

	int strategySize = 3 * sizeof(float) * strategy.size();

	/*
	sizeof(uint8_t) * 4:	required for storage of :
					- node identifier "c" for cfr updates (1)
					- number of actions for cfr updates (1)
					- number of children for tree traversal (1)
					- boolean that is true when player 1 has action / false otherwise.
	
	sizeof(byte*)	: required for storage of:
					- starting offset for children.
	*/
	int nodeSize = strategySize + (4 * sizeof(uint8_t)) + sizeof(byte*);

	//Update SizeAtDepth for future tree construction.
	mSizeAtDepth->at(depth) += nodeSize;

	//Recursively find the size of children to add to total sum.
	int childrenTotalSize = 0;
	
	GameNodeChildren *pGameNodeChildren = mChildrenFromGameFunc(gameState, strategy);

	using ChildGameNodes = GameNodeChildrenGamesNodes<GameState, Action>;
	using ChildChanceNodes = GameNodeChildrenChanceNodes<ChanceNode, Action>;
	using ChildTerminalNodes = GameNodeChildrenTerminalNodes<Action>;
	using ActionsToChild = std::vector<Action*>;


	//Iterate over all children that are gamestates.
	ChildGameNodes *childGameNodes = pGameNodeChildren->GetChildGameNodes();

	using ChildGameNode = GameNodeChildGameNode<GameState, Action>;
	

	typename std::vector<ChildGameNode*>::iterator iChildGameNode;
	typename std::vector<ChildGameNode*>::iterator iChildGameNodeEnd;
	iChildGameNode = childGameNodes->IterBegin();
	iChildGameNodeEnd = childGameNodes->IterEnd();

	for (iChildGameNode; iChildGameNode < iChildGameNodeEnd; iChildGameNode++) {
		ChildGameNode* pChildGameNode = *iChildGameNode;
		GameState childGameState = pChildGameNode->GetGameState();
		Strategy childStrategy = pChildGameNode->GetStrategy();
		ActionsToChild parentActions = pChildGameNode->GetActionsToChild();

		int numActionsToChild = parentActions.size();
		int newUniqueHistoryCnt = numActionsToChild * uniqueHistoriesCnt;
		childrenTotalSize += PreProcessorHelper(childGameState, childStrategy, depth + 1, newUniqueHistoryCnt);
	}

	//Iterate over all children that are chance nodes.
	ChildChanceNodes* childChanceNodes = pGameNodeChildren->GetChildChanceNodes();

	using ChildChanceNode = GameNodeChildChanceNode<ChanceNode, Action>;

	typename std::vector<ChildChanceNode*>::iterator iChildChanceNode;
	typename std::vector<ChildChanceNode*>::iterator iChildChanceNodeEnd;
	iChildChanceNode = childChanceNodes->IterBegin();
	iChildChanceNodeEnd = childChanceNodes->IterEnd();

	for (iChildChanceNode; iChildChanceNode < iChildChanceNodeEnd; iChildChanceNode++) {
		ChildChanceNode* pChildChanceNode = *iChildChanceNode;
		ChanceNode childChanceNode = pChildChanceNode->GetChanceNode();
		ActionsToChild parentActions = pChildChanceNode->GetActionsToChild();

		int numActionsToChild = parentActions.size();
		int newUniqueHistoryCnt = numActionsToChild * uniqueHistoriesCnt;
		childrenTotalSize += PreProcessorHelper(childChanceNode, depth + 1, newUniqueHistoryCnt);
	}

	//Iterate over all children that are terminal nodes.
	ChildTerminalNodes* childTerminalNodes = pGameNodeChildren->GetChildTerminalNodes();

	using ChildTerminalNode = GameNodeChildTerminalNode<Action>;

	typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNode;
	typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNodeEnd;
	iChildTerminalNode = childTerminalNodes->IterBegin();
	iChildTerminalNodeEnd = childTerminalNodes->IterEnd();

	for (iChildTerminalNode; iChildTerminalNode < iChildTerminalNodeEnd; iChildTerminalNode++) {
		ChildTerminalNode* pChildTerminalNode = *iChildTerminalNode;
		ActionsToChild parentActions = pChildTerminalNode->GetActionsToChild();

		int numActionsToChild = parentActions.size();
		int newUniqueHistoryCnt = numActionsToChild * uniqueHistoriesCnt;
		childrenTotalSize += PreProcessorHelperTerminal(depth + 1, newUniqueHistoryCnt);
	}
	//Free memory on the heap
	delete pGameNodeChildren;

	
	return nodeSize + childrenTotalSize;
}

/**
* @brief  A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of the required information from a Terminal Node, which has no descendants.
*/
template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>
::PreProcessorHelperTerminal(int depth, int uniqueHistoriesCnt){
	
	long terminalNodeSize = sizeof(float) + sizeof(byte);
	long totalTerminalNodesSize = terminalNodeSize * uniqueHistoriesCnt;
	//Update SizeAtDepth for future tree construction.
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(totalTerminalNodesSize);
	}
	else {
		mSizeAtDepth->at(depth) += totalTerminalNodesSize;
	}
	return totalTerminalNodesSize;
}

/**
* @brief  A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of current Chance Node and all of its descendants in bytes.
*/
template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>
::PreProcessorHelper(ChanceNode chanceNode, int depth, int uniqueHistoriesCnt) {
	
	//Ensure mSizeAtDepth is large enough to store value at current depth
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(0);
	}

	ChanceNodeChildren * pChanceNodeChildren = mChildrenFromChanceFunc(chanceNode);

	/*
	Total size for float vector of probability of reaching each child node.
	*/
	long probabilityCnt = pChanceNodeChildren->size();
	long probabilitySize = probabilityCnt * (long) sizeof(float);

	/*
	sizeof(uint8_t) * 2:	required for storage of :
					- node identifier "c" for deserialization (1)
					- number of children for tree traversal (1)
	 
	sizeof(byte*)	: required for storage of:
					- starting offset for children.
	*/
	int nodeSize = probabilitySize + ( 2 * sizeof(uint8_t) ) + sizeof(long);;

	//Update SizeAtDepth for future tree construction.
	mSizeAtDepth->at(depth) += nodeSize;

	//Recursively find the size of children to add to total sum.
	long childrenTotalSize = 0;


	using ChildGameNodes = ChanceNodeChildrenGamesNodes<GameState, Action>;
	using ChildTerminalNodes = ChanceNodeChildrenTerminalNodes;
	using ActionsToChild = std::vector<Action*>;

	//Iterate over all children that are gamestates.
	ChildGameNodes* childGameNodes = pChanceNodeChildren->GetChildGameNodes();
	
	using ChildGameNode = ChanceNodeChildGameNode<GameState, Action>;

	typename std::vector<ChildGameNode*>::iterator iChildGameNode;
	typename std::vector<ChildGameNode*>::iterator iChildGameNodeEnd;
	iChildGameNode = childGameNodes->IterBegin();
	iChildGameNodeEnd = childGameNodes->IterEnd();

	for (iChildGameNode; iChildGameNode < iChildGameNodeEnd; iChildGameNode++) {
		ChildGameNode* pChildGameNode = *iChildGameNode;
		GameState childGameState = pChildGameNode->GetGameState();
		Strategy childStrategy = pChildGameNode->GetStrategy();

		childrenTotalSize += PreProcessorHelper(childGameState, childStrategy, depth + 1, uniqueHistoriesCnt);
	}

	//Iterate over all children that are terminal nodes.
	ChildTerminalNodes* childTerminalNodes = pChanceNodeChildren->GetChildTerminalNodes();

	using ChildTerminalNode = ChanceNodeChildTerminalNode;

	typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNode;
	typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNodeEnd;
	iChildTerminalNode = childTerminalNodes->IterBegin();
	iChildTerminalNodeEnd = childTerminalNodes->IterEnd();

	for (iChildTerminalNode; iChildTerminalNode < iChildTerminalNodeEnd; iChildTerminalNode++) {

		childrenTotalSize += PreProcessorHelperTerminal(depth + 1, uniqueHistoriesCnt);
	}

	//Free memory on heap
	delete pChanceNodeChildren;

	return nodeSize + childrenTotalSize;
}


/**
* @brief Preprocess GameTree before construction
* @return Size of entire game tree in bytes
*/
template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>
::PreProcessor() {
	//Initialize sizeAtDepth vector for future tree construction.
	mSizeAtDepth = new std::vector<long>();
	long treeSize = PreProcessorHelper(mStartingChanceNode, 0, 1);

	/*Iterate through updated sizeAtDepth vector for cumulative depth offset.
	  This is essential for future tree construction via Depth First Search.
	*/
	long cumulativeDepthOffset = 0;
	mCumulativeOffsetAtDepth = new std::vector<long>(mSizeAtDepth->size(), 0);
	for (int iSizeAtDepth = 1; iSizeAtDepth < mSizeAtDepth->size(); iSizeAtDepth++) {
		long depthSize = (long) mSizeAtDepth->at(iSizeAtDepth - 1);
		cumulativeDepthOffset += depthSize;
		mCumulativeOffsetAtDepth->at(iSizeAtDepth) = cumulativeDepthOffset;
	}
	return treeSize;
}


/*GAME TREE CONSTRUCTOR DEFINITIONS*/


/**
 * @brief Sets current GameNode and metadata in gameTree and recursively sets its children.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a 
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param pCurrNode 
 * @param depth 
 */

template<typename GameState, typename ChanceNode, typename Action >
inline void CFRGameTree<GameState, ChanceNode, Action>
::TreeConstructorHelper(GameState gameState, Strategy strategy, int depth, GameHistory* pHistory, bool setNode) {
	
	GameNodeChildren* pGameNodeChildren = mChildrenFromGameFunc(gameState, strategy);
	//Sets node in Game Tree if setNode is true. Used to avoid duplicate nodes.
	if (setNode) {
		//Obtain offset using cumulativeOffsetAtDepth as part of Depth First Search tree construction.
		long offset = mCumulativeOffsetAtDepth->at(depth);
		long initialOffset = offset;

		//Set classifier "g" at offset to indicate that this node is a gamenode
		mGameTree[offset++] = 'g';

		//Set uint_8 for determing player who has action (1 : player 1 ; -1 : player 2)
		if (mPlayerOneHasAction(gameState)) {
			mGameTree[offset++] = (int8_t) 1;
		}
		else {
			mGameTree[offset++] = (int8_t) -1;
		}

		//Stores number of actions
		int numActions = strategy.size();
		mGameTree[offset++] = (uint8_t) numActions;

		
		//Stores floating point values for cumulative regret, strategy probabilities, and current regret.
		float uniform_prob = 1.0 / ( (float) numActions );
		int totalActionsSize = numActions * sizeof(float);
		for (long iFloatOffset = offset; iFloatOffset < offset + totalActionsSize; iFloatOffset += sizeof(float)) {
			SetFloatAtBytePtr(mGameTree + iFloatOffset, uniform_prob);
		}
		offset += totalActionsSize;
		for (long iFloatOffset = offset; iFloatOffset < offset + ( 2 * totalActionsSize ); iFloatOffset += sizeof(float)) {
			SetFloatAtBytePtr(mGameTree + iFloatOffset, 0.0f);
		}
		offset += 2 * totalActionsSize;

		//Stores starting offset of children.
		mGameTree[offset] = mCumulativeOffsetAtDepth->at(depth + 1);
		offset += sizeof(long);

		//Update cumulativeOffsetAtDepth for Depth First Search tree construction.
		long offsetDiff = offset - initialOffset;
		mCumulativeOffsetAtDepth->at(depth) += offsetDiff;
	}

	using ChildGameNodes = GameNodeChildrenGamesNodes<GameState, Action>;
	using ChildChanceNodes = GameNodeChildrenChanceNodes<ChanceNode, Action>;
	using ChildTerminalNodes = GameNodeChildrenTerminalNodes<Action>;
	using ActionsToChild = std::vector<Action*>;

	/*Iterate over all children that are gamestates and find number of children.
	For each action*/
	ChildGameNodes* childGameNodes = pGameNodeChildren->GetChildGameNodes();

	using ChildGameNode = GameNodeChildGameNode<GameState, Action>;


	typename std::vector<ChildGameNode*>::iterator iChildGameNode;
	typename std::vector<ChildGameNode*>::iterator iChildGameNodeEnd;
	iChildGameNode = childGameNodes->IterBegin();
	iChildGameNodeEnd = childGameNodes->IterEnd();

	for (iChildGameNode; iChildGameNode < iChildGameNodeEnd; iChildGameNode++) {
		

		ChildGameNode* pChildGameNode = *iChildGameNode;
		GameState childGameState = pChildGameNode->GetGameState();
		Strategy childStrategy = pChildGameNode->GetStrategy();
		ActionsToChild parentActions = pChildGameNode->GetActionsToChild();

		bool isFirstChild = true;
		typename std::vector<Action*>::iterator iAction;
		typename std::vector<Action*>::iterator iActionEnd;
		iAction = parentActions.begin();
		iActionEnd = parentActions.end();

		for (iAction; iAction < iActionEnd; iAction++) {
			Action* pAction = *iAction;

			//Add History node for game state and each action to history for next level of Game Tree
			GameHistory *pNewHistory = pHistory->AddToNewHistory(gameState, pAction);

			bool setChild = false;
			//For first recursive call, use bool for next setNode as setNode && true.
			if (isFirstChild) {
				setChild = setNode;
				isFirstChild = false;
			}
			TreeConstructorHelper(childGameState, childStrategy, depth + 1, pNewHistory, setChild);
			delete pNewHistory;
			
		}
	}

	//Iterate over all children that are chance nodes.
	ChildChanceNodes* childChanceNodes = pGameNodeChildren->GetChildChanceNodes();

	using ChildChanceNode = GameNodeChildChanceNode<ChanceNode, Action>;

	typename std::vector<ChildChanceNode*>::iterator iChildChanceNode;
	typename std::vector<ChildChanceNode*>::iterator iChildChanceNodeEnd;
	iChildChanceNode = childChanceNodes->IterBegin();
	iChildChanceNodeEnd = childChanceNodes->IterEnd();

	for (iChildChanceNode; iChildChanceNode < iChildChanceNodeEnd; iChildChanceNode++) {
		ChildChanceNode* pChildChanceNode = *iChildChanceNode;
		ChanceNode childChanceNode = pChildChanceNode->GetChanceNode();
		ActionsToChild parentActions = pChildChanceNode->GetActionsToChild();

		bool isFirstChild = true;
		typename std::vector<Action*>::iterator iAction;
		typename std::vector<Action*>::iterator iActionEnd;
		iAction = parentActions.begin();
		iActionEnd = parentActions.end();

		for (iAction; iAction < iActionEnd; iAction++) {
			Action* pAction = *iAction;

			//Add History node for game state and each action to history for next level of Game Tree
			GameHistory* pNewHistory = pHistory->AddToNewHistory(gameState, pAction);

			bool setChild = false;
			//For first recursive call, use bool for next setNode as setNode && true.
			if (isFirstChild) {
				setChild = setNode;
				isFirstChild = false;
			}
			TreeConstructorHelper(childChanceNode, depth + 1, pNewHistory, setChild);
			delete pNewHistory;

		}
	}

	//Iterate over all children that are terminal nodes.
	ChildTerminalNodes* childTerminalNodes = pGameNodeChildren->GetChildTerminalNodes();

	using ChildTerminalNode = GameNodeChildTerminalNode<Action>;

	typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNode;
	typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNodeEnd;
	iChildTerminalNode = childTerminalNodes->IterBegin();
	iChildTerminalNodeEnd = childTerminalNodes->IterEnd();

	for (iChildTerminalNode; iChildTerminalNode < iChildTerminalNodeEnd; iChildTerminalNode++) {
		ChildTerminalNode* pChildTerminalNode = *iChildTerminalNode;
		ActionsToChild parentActions = pChildTerminalNode->GetActionsToChild();

		bool isFirstChild = true;
		typename std::vector<Action*>::iterator iAction;
		typename std::vector<Action*>::iterator iActionEnd;
		iAction = parentActions.begin();
		iActionEnd = parentActions.end();

		for (iAction; iAction < iActionEnd; iAction++) {
			Action* pAction = *iAction;

			//Add History node for game state and each action to history for next level of Game Tree
			GameHistory* pNewHistory = pHistory->AddToNewHistory(gameState, pAction);

			TreeConstructorHelperTerminal(depth + 1, pNewHistory);
			delete pNewHistory;
		}
	}
	//Free memory on the heap
	delete pGameNodeChildren;
	
	return;
}

/**
 * @brief Sets current TerminalNode and metadata in gameTree.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param pCurrNode
 * @param depth
 */
template<typename GameState, typename ChanceNode, typename Action >
inline void CFRGameTree<GameState, ChanceNode, Action>
::TreeConstructorHelperTerminal(int depth, GameHistory* pHistory) {
	
	long offset = mCumulativeOffsetAtDepth->at(depth);
	int initialOffset = offset;

	//Set classifier to "t" to identify that the node is a Terminal Node
	mGameTree[offset] = 't';
	offset++;

	//Find utility value at terminal node using history.
	float utilityVal = mUtilityFunc(pHistory);
	//Stores utility for terminal node for player 1
	SetFloatAtBytePtr(mGameTree + offset, utilityVal);
	offset += sizeof(float);

	//Update Cumulative Offset At Depth for Depth First Search Tree Construction.
	long offsetDiff = offset - initialOffset;
	mCumulativeOffsetAtDepth->at(depth) += offsetDiff;
	
}

/**
 * @brief Sets current ChanceNode and metadata in gameTree and recursively sets its children.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param pCurrNode
 * @param depth
 */
template< typename GameState, typename ChanceNode, typename Action >
inline void CFRGameTree<GameState, ChanceNode, Action>
::TreeConstructorHelper(ChanceNode chanceNode, int depth, GameHistory* pHistory, bool setNode) {
	if (setNode) {
		long offset = mCumulativeOffsetAtDepth->at(depth);
		int initialOffset = offset;

		//Set classifier to "c" to identify that the node is a Chance Node
		mGameTree[offset++] = 'c';

		ChanceNodeChildren* pChildrenNodes = mChildrenFromChanceFunc(chanceNode);

		//Stores number of children                                                                                                                                                                                                            
		int numChildren = pChildrenNodes->size();
		mGameTree[offset++] = (uint8_t) numChildren;

		//Stores starting offset for children
		mGameTree[offset] = mCumulativeOffsetAtDepth->at(depth + 1);
		offset += (long) sizeof(long);

	
		//Add History node to history for next level of Game Tree
		GameHistory *pNewHistory = pHistory->AddToNewHistory(chanceNode);

		/*Recursively call Tree Constructor Helper on each child at next depth.
		  Store probability of reaching each child
		*/

		using ChildGameNodes = ChanceNodeChildrenGamesNodes<GameState, Action>;
		using ChildTerminalNodes = ChanceNodeChildrenTerminalNodes;

		//Iterate over all children that are gamestates.
		ChildGameNodes* childGameNodes = pChildrenNodes->GetChildGameNodes();

		using ChildGameNode = ChanceNodeChildGameNode<GameState, Action>;

		typename std::vector<ChildGameNode*>::iterator iChildGameNode;
		typename std::vector<ChildGameNode*>::iterator iChildGameNodeEnd;
		iChildGameNode = childGameNodes->IterBegin();
		iChildGameNodeEnd = childGameNodes->IterEnd();

		for (iChildGameNode; iChildGameNode < iChildGameNodeEnd; iChildGameNode++) {
			ChildGameNode* pChildGameNode = *iChildGameNode;
			GameState childGameState = pChildGameNode->GetGameState();
			Strategy childStrategy = pChildGameNode->GetStrategy();
			float probToChild = pChildGameNode->GetProbToChild();
			if (setNode) {

				SetFloatAtBytePtr(mGameTree + offset, probToChild);
				offset += sizeof(float);
			}

			TreeConstructorHelper(childGameState, childStrategy, depth + 1, pNewHistory, setNode);
		}

		//Iterate over all children that are terminal nodes.
		ChildTerminalNodes* childTerminalNodes = pChildrenNodes->GetChildTerminalNodes();

		using ChildTerminalNode = ChanceNodeChildTerminalNode;

		typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNode;
		typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNodeEnd;
		iChildTerminalNode = childTerminalNodes->IterBegin();
		iChildTerminalNodeEnd = childTerminalNodes->IterEnd();

		for (iChildTerminalNode; iChildTerminalNode < iChildTerminalNodeEnd; iChildTerminalNode++) {
			ChildTerminalNode* pChildTerminalNode = *iChildTerminalNode;
			float probToChild = pChildTerminalNode->GetProbToChild();
			if (setNode) {
				SetFloatAtBytePtr(mGameTree + offset, probToChild);
				offset += sizeof(float);
			}

			TreeConstructorHelperTerminal(depth + 1, pNewHistory);
		}
		//Free space on heap
		delete pNewHistory;
		delete pChildrenNodes;

		//Update Cumulative Offset At Depth for Depth First Search Tree Construction
		long offsetDiff = offset - initialOffset;
		mCumulativeOffsetAtDepth->at(depth) += offsetDiff;
	}
	return;
}


/**
 * @brief Calls Tree Constructor Helper on starting Chance node (the root of the tree).
 *		  Sets values for CumulativeOffsetAtDepth that continously updates throughout
 *		  the traversal of the tree.
		  This creates a tree structure that is organized by the depth of nodes.
 */
template< typename GameState, typename ChanceNode, typename Action >
inline void CFRGameTree<GameState, ChanceNode, Action>
::ConstructTree() {
	int treeSize = PreProcessor();
	mGameTree = new byte[treeSize];
	GameHistory *pStartingHistory = new GameHistory();

	TreeConstructorHelper(mStartingChanceNode, 0, pStartingHistory, true);
	delete pStartingHistory;
}


template< typename GameState, typename ChanceNode, typename Action >
inline std::pair<float, long> CFRGameTree<GameState, ChanceNode, Action>
::CFRHelper(long pNodePos, float reachProbPlayerOne, float reachProbPlayerTwo) {
	unsigned char nodeIdentifier = (unsigned char) mGameTree[pNodePos];
	if (nodeIdentifier == 'g') {
		TreeGameNode treeGameNode(mGameTree, pNodePos);
		return CFRGameNodeHelper(&treeGameNode, reachProbPlayerOne, reachProbPlayerTwo);
	}
	if (nodeIdentifier == 't') {
		float utilityVal = mGameTree[pNodePos + 1];
		long pNextNodePos = pNodePos + sizeof(byte) + sizeof(float);
		return std::pair<float, long>(utilityVal, pNextNodePos);
	}
	if (nodeIdentifier == 'c') {
		TreeChanceNode treeChanceNode(mGameTree, pNodePos);
		return CFRChanceNodeHelper(&treeChanceNode, reachProbPlayerOne, reachProbPlayerTwo);
	}
	return std::pair<float, long>(0.0f, 0);
	
	
}

/**
 * @brief Calculates utility for gamenode at pGameNodePos by recursively calling on its children.
 * @param pGameNodePos 
 * @param reachProbPlayerOne 
 * @param reachProbPlayerTwo 
 * @return Pair where first element = utility of gamenode, second element = position of next node in tree.
 */
template< typename GameState, typename ChanceNode, typename Action >
inline std::pair<float, long> CFRGameTree<GameState, ChanceNode, Action>
::CFRGameNodeHelper(TreeGameNode* treeGameNode, float reachProbPlayerOne, float reachProbPlayerTwo) {
		
	long pChildPos = treeGameNode->mpChildStartOffset;
	std::vector<float> childrenUtilities(treeGameNode->mNumChildren);
	float val = 0;
	int playerTurn = treeGameNode->mPlayerToAct;
	for (int iChild = 0; iChild < treeGameNode->mNumChildren; iChild++) {
		float childReachProbOne = 1.0;
		float childReachProbTwo = 1.0;
		float currStratProb = treeGameNode->GetCurrStratProb(iChild);
		if (treeGameNode->mPlayerToAct == 1) {
			childReachProbOne = reachProbPlayerOne * currStratProb;
		}
		/*If it isn't player 1's turn, it is player 2's turn*/
		else {
			childReachProbTwo = reachProbPlayerTwo * currStratProb;
		}

		std::pair<float, long> childReturnPair = CFRHelper(pChildPos, childReachProbOne, childReachProbTwo);
		float childUtility = childReturnPair.first;
		pChildPos = childReturnPair.second;

		val += currStratProb * childUtility;
		childrenUtilities.at(iChild) = childUtility;
	}
	float cfrReach;
	float reach;
	if (treeGameNode->mPlayerToAct == 1) {
		cfrReach = reachProbPlayerTwo;
		reach = reachProbPlayerOne;
	}
	/*If it isn't player 1's turn, it is player 2's turn*/
	else {
		cfrReach = reachProbPlayerOne;
		reach = reachProbPlayerTwo;
	}
	for (int iChild = 0; iChild < treeGameNode->mNumChildren; iChild++) {
		float regretMultiplier = (float) treeGameNode->mPlayerToAct;
		float childUtility = childrenUtilities.at(iChild);
		float currStratProb = treeGameNode->GetCurrStratProb(iChild);
		float actionCfrRegret = regretMultiplier * cfrReach * (childUtility - val);
		treeGameNode->AddCumRegret(actionCfrRegret, iChild);
		treeGameNode->AddCumStratProb(reach * currStratProb, iChild);
	}
	return std::pair<float, long>(val, treeGameNode->mpNextNodePos);

}

/**
 * @brief Calculates utility for chancenode at pChanceNodePos by recursively calling on its children.
 * @param pChanceNodePos 
 * @param reachProbPlayerOne 
 * @param reachProbPlayerTwo 
 * @return Pair where first element = utility of chance node, second element = position of next node in tree.
 */
template< typename GameState, typename ChanceNode, typename Action >
inline std::pair<float, long> CFRGameTree<GameState, ChanceNode, Action>
::CFRChanceNodeHelper(TreeChanceNode *treeChanceNode, float reachProbPlayerOne, float reachProbPlayerTwo) {
	
	float cfrRecursiveVal = 0;
	long pChildPos = treeChanceNode->mpChildStartOffset;
	for (int iChild = 0; iChild < treeChanceNode->mNumChildren; iChild++) {
		std::pair<float, long> childReturnPair = CFRHelper(pChildPos, reachProbPlayerOne, reachProbPlayerTwo);
		float childReachProb = treeChanceNode->GetChildReachProb(iChild);
		cfrRecursiveVal += childReachProb * childReturnPair.first;
		pChildPos = childReturnPair.second;
	}
	return std::pair<float, long>(cfrRecursiveVal, treeChanceNode->mpNextNodePos);
}

template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>
::UpdateStratProbsRecursive(long pNodePos) {
	unsigned char nodeIdentifier = mGameTree[pNodePos];
	if (nodeIdentifier == 'g') {
		TreeGameNode treeGameNode(mGameTree, pNodePos);;
		return UpdateStratGameNode(&treeGameNode);
	}
	if (nodeIdentifier == 'c') {
		TreeChanceNode treeChanceNode(mGameTree, pNodePos);
		return UpdateStratChanceNode(&treeChanceNode);
	}
	if (nodeIdentifier == 't') {
		return pNodePos + sizeof(byte) + sizeof(float);
	}
	return -1.0f;
	
}

template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>
::UpdateStratGameNode(TreeGameNode *treeGameNode) {
	
	int numChildren = treeGameNode->mNumChildren;
	int numActions = treeGameNode->mNumActions;
	float regretSum = 0;
	for (int iAction = 0; iAction < numChildren; iAction++) {
		float actionCumRegret = treeGameNode->GetCumRegret(iAction);
		if (actionCumRegret > 0) {
			regretSum += actionCumRegret;
		}
	}
	for (int iAction = 0; iAction < numChildren; iAction++) {
		float cumRegret = treeGameNode->GetCumRegret(iAction);
		float updatedProb = regretSum > 0 ? (std::max(cumRegret, 0.0f) / regretSum) : (1.0f / (float) numActions);
		treeGameNode->SetCurrStratProb(updatedProb, iAction);
	}
	
	long pChildPos = treeGameNode->mpChildStartOffset;
	for (int iChild = 0; iChild < numChildren; iChild++) {
		pChildPos = UpdateStratProbsRecursive(pChildPos);
	}
	return treeGameNode->mpNextNodePos;
}

template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>
::UpdateStratChanceNode(TreeChanceNode *treeChanceNode) {

	long pChildPos = treeChanceNode->mpChildStartOffset;
	for (int iChild = 0; iChild < treeChanceNode->mNumChildren; iChild++) {
		pChildPos = UpdateStratProbsRecursive(pChildPos);
	}
	return treeChanceNode->mpNextNodePos;
}



template< typename GameState, typename ChanceNode, typename Action >
inline void CFRGameTree<GameState, ChanceNode, Action>
::CFR(int iterations, float accuracy) {
	
	for (int iCFR = 0; iCFR < iterations; iCFR++) {
		CFRHelper(0, 1.0, 1.0);
		//std::cout << "Iteration: " << iCFR << "\n";
		//std::cout << "Before strat update\n";
		//PrintGameTree();
		
		UpdateStratProbsRecursive(0);
		//std::cout << "\nAfter strat update:\n\n";
		//PrintGameTree();
	}
	UpdateNashStrategy(0, iterations);
	return;
}

template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>
::UpdateNashStrategy(long pNodePos, int numIterations) {
	unsigned char nodeIdentifier = (unsigned char) mGameTree[pNodePos];

	if (nodeIdentifier == 'g') {

		TreeGameNode treeGameNode(mGameTree, pNodePos);
		int numChildren = treeGameNode.mNumChildren;
		int numActions = treeGameNode.mNumActions;
		float unnormalizedTotal = 0;
		for (int iAction = 0; iAction < numChildren; iAction++) {
			float cumStratProb = treeGameNode.GetCumStratProb(iAction);
			float avgStratProb = cumStratProb / (float) numIterations;
			treeGameNode.SetCurrStratProb(avgStratProb, iAction);
			unnormalizedTotal += avgStratProb;
		}
		float normalizeMultiplier = 1.0f / unnormalizedTotal;
		for (int iAction = 0; iAction < numChildren; iAction++) {
			float unNormalizedStratProb = treeGameNode.GetCurrStratProb(iAction);
			float normalizedStratProb = unNormalizedStratProb * normalizeMultiplier;
			treeGameNode.SetCurrStratProb(normalizedStratProb, iAction);
		}
		long pChildPos = treeGameNode.mpChildStartOffset;
		for (int iChild = 0; iChild < numChildren; iChild++) {
			pChildPos = UpdateNashStrategy(pChildPos, numIterations);
		}
		return treeGameNode.mpNextNodePos;
	}
	if (nodeIdentifier == 't') {
		return pNodePos + sizeof(byte) + sizeof(float);
	}
	if (nodeIdentifier == 'c') {
		TreeChanceNode treeChanceNode(mGameTree, pNodePos);
		long pChildPos = treeChanceNode.mpChildStartOffset;
		for (int iChild = 0; iChild < treeChanceNode.mNumChildren; iChild++) {
			pChildPos = UpdateNashStrategy(pChildPos, numIterations);
		}

		return treeChanceNode.mpNextNodePos;
	}
	return 0;
}






template< typename GameState, typename ChanceNode, typename Action >
inline void CFRGameTree<GameState, ChanceNode, Action>::PrintGameTree() {
	PrintTreeHelper(0);

}

template< typename GameState, typename ChanceNode, typename Action >
inline long CFRGameTree<GameState, ChanceNode, Action>::PrintTreeHelper(long pNodePos) {
	
	unsigned char nodeIdentifier = mGameTree[pNodePos];
	if (nodeIdentifier == 't') {
		return pNodePos + sizeof(byte) + sizeof(float);
	}
	else if (nodeIdentifier == 'g') {
		TreeGameNode treeGameNode(mGameTree, pNodePos);
		std::cout << treeGameNode;
		
		long pChildPos = treeGameNode.mpChildStartOffset;
		for (int iChild = 0; iChild < treeGameNode.mNumChildren; iChild++) {
			pChildPos = PrintTreeHelper(pChildPos);
		}
		return treeGameNode.mpNextNodePos;
	}
	else {
		TreeChanceNode treeChanceNode(mGameTree, pNodePos);
		

		long pChildPos = treeChanceNode.mpChildStartOffset;
		for (int iChild = 0; iChild < treeChanceNode.mNumChildren; iChild++) {
			pChildPos = PrintTreeHelper(pChildPos);
		}
		return treeChanceNode.mpNextNodePos;
	}
	
}










