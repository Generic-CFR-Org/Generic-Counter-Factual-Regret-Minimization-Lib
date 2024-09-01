#pragma once
#include "pch.h"
#include "framework.h"
#include "cfrGameTreeUtils.h"


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

template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
class CFRGameTree {

public:

	/*typedef std::vector<Action*> Strategy;

	typedef HistoryNode<GameState, ChanceNode, Action> GameHistoryNode;
	typedef std::vector<GameHistoryNode> History;

	typedef HistoryTree<GameState, ChanceNode, Action> GameHistoryTree;
	typedef HistoryTreeNode<GameState, ChanceNode, Action> GameHistoryTreeNode;

	using GameNodeChildren = ChildrenFromGameNode<GameState, ChanceNode, Action>;
	using ChanceNodeChildren = ChildrenFromChanceNode<GameState, Action>;*/




	CFRGameTree() {

		mpGameInfo = nullptr;
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
		GameInfo* gameInfo,
		GameNodeChildren* ( *childNodesFromGameNode )( GameState, Strategy, GameInfo* ),
		ChanceNodeChildren* ( *childNodesFromChanceNode ) ( ChanceNode, GameInfo* ),
		bool ( *playerOneHasAction ) ( GameState, GameInfo* ),
		ChanceNode startingChanceNode,
		float ( *utilityFromHistory )( History, GameInfo* )
	) {
		mpGameInfo = gameInfo;
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
	void CFR(int);
	void CFRAccuracy(float);
	void PrintGameTree();


private:


	/*INSTANCE MEMBERS*/

	/*Pointer to constructed Game traversal tree*/
	byte* mGameTree;

	/*Pointer to Constructed Regret Table for information sets.*/
	byte* mRegretTable;

	/*Pointer to User defined Game Info used to generate the tree*/
	GameInfo* mpGameInfo;

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
	GameNodeChildren* (*mChildrenFromGameFunc)( GameState gameState, Strategy strategy, GameInfo* gameInfo);

	/**
	 * @brief User defined function returns 2-tuple of list pointers.
	 *		  1st elem is a list of Child Game Nodes with probability from the Chance Node.
	 *		  2nd elem is a list of Child Terminal Nodes with probability from the Chance Node.
	 */
	ChanceNodeChildren* (*mChildrenFromChanceFunc) ( ChanceNode, GameInfo* gameInfo );

	/**
	 * @brief User defined function that gets player who has action for a given gamestate
	 * @param pCurrGameState 
	 * @return true if player 1 has action, false if player 2 has action.
	 */
	bool (* mPlayerOneHasAction) (GameState currGameState, GameInfo* gameInfo);
	
	/**
	 * @brief User defined function that gets utility value from given Terminal Node.
	 * @param pCurrTerminalNode
	 * @return (float) value of utility of pCurrTerminalNode.
	 */
	float ( *mUtilityFunc ) ( History history, GameInfo* gameInfo);

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
	void TreeConstructorHelper(GameState gameState, Strategy strategy, int depth, GameHistoryTreeNode* pHistoryTreeNode);
	void TreeConstructorHelperTerminal(int depth, GameHistoryTreeNode* pHistoryTreeNode);
	void TreeConstructorHelper(ChanceNode chanceNode, int depth, GameHistoryTreeNode* pHistoryTreeNode);
	
	float CFRHelper(TreeGameNode *treeGameNode, float reachProbPlayerOne, float reachProbPlayerTwo, int actionIndex);
	float CFRHelper(TreeChanceNode *treeChanceNode, float reachProbPlayerOne, float reachProbPlayerTwo, int actionIndex);
	
	void UpdateStratProbs(TreeGameNode* treeGameNode);
	void UpdateStratProbs(TreeChanceNode* treeChanceNode);
	
	void UpdateNashStrategy(TreeGameNode* treeGameNode, int numIterations);
	void UpdateNashStrategy(TreeChanceNode* treeChanceNode, int numIterations);
	
	/**
	 * @brief Helper function for printing out strategy for each Game Node in
	 *	      the game Tree.
	 * @param pNodePos 
	 * @return (long) offset of next node to evaluate.
	 */
	void PrintTreeHelper(TreeGameNode* treeGameNode);
	void PrintTreeHelper(TreeChanceNode* treeChanceNode);
	
};

/**
* @brief A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of the required information from a GameNode and all of its descendants in bytes.
*/
template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline long CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::PreProcessorHelper(GameState gameState, Strategy strategy, int depth, int uniqueHistoriesCnt) {

	//Ensure sizeAtDepth is large enough to store value at current depth
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(0);
	}

	/*
	Strategy size =

	*	number of actions * sizeof(float) * 3
		Multiplying by 3 is the result of needing to store:
			- Current Strategy
			- Cumulative sum of strategies
			- Immediate counterfactual regret for each action.

	* + (num of actions * sizeof(uint8_t))
			- Stores number of children per action due to many to many relationship.
	*/
	int numActions = strategy.size();
	int strategySize = 3 * sizeof(float) * numActions;

	GameNodeChildren* pGameNodeChildren = mChildrenFromGameFunc(gameState, strategy, mpGameInfo);
	int numGameChildren = pGameNodeChildren->GameSize();
	int numChanceChildren = pGameNodeChildren->ChanceSize();
	int numTerminalChildren = pGameNodeChildren->TerminalSize();
	int numTotalChildren = numGameChildren + numChanceChildren + numTerminalChildren;
	/*
	sizeof(uint8_t) * 5:
					- boolean that is true when player 1 has action / false otherwise.
					- number of actions for cfr updates (1)
					- number of game node children for tree traversal (1)
					- number of chance node children for tree traversal (1)
					- number of terminal node children for tree travesal (1).

	sizeof(byte*)	:
					- starting offset for children.

	sizeof(uint8_t) * numTotalChildren:
					- Many actions -> one child relationship -> num Actions per child.
					* Used for using child to parent action index array.

	*/
	int nodeSize = strategySize;
	nodeSize += ( 5 * sizeof(uint8_t) );
	nodeSize += sizeof(long);
	nodeSize += sizeof(uint8_t) * numTotalChildren;

	/*Recursively find the size of children to add to total sum.
	As well as to find the relationship between action and child.*/
	int OneChildToManyActionSize = 0;
	int childrenTotalSize = 0;

	using ChildGameNodes = GameNodeChildrenGamesNodes<GameState, Action>;
	using ChildChanceNodes = GameNodeChildrenChanceNodes<ChanceNode, Action>;
	using ChildTerminalNodes = GameNodeChildrenTerminalNodes<Action>;
	using ActionsToChild = std::vector<Action*>;


	//Iterate over all children that are gamestates.
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

		//Required for storage of child to parent action arr.
		OneChildToManyActionSize += parentActions.size() * sizeof(uint8_t);

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

		//Required for storage of child to parent action arr.
		OneChildToManyActionSize += parentActions.size() * sizeof(uint8_t);

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

		//Required for storage of child to parent action arr.
		OneChildToManyActionSize += parentActions.size() * sizeof(uint8_t);

		int numActionsToChild = parentActions.size();
		int newUniqueHistoryCnt = numActionsToChild * uniqueHistoriesCnt;
		childrenTotalSize += PreProcessorHelperTerminal(depth + 1, newUniqueHistoryCnt);
	}

	//Free memory on the heap
	delete pGameNodeChildren;

	nodeSize += OneChildToManyActionSize;

	//Update SizeAtDepth for future tree construction.
	mSizeAtDepth->at(depth) += nodeSize;

	/*std::cout << "Game Node at depth " << depth << " size: " << nodeSize << "\n";*/
	return nodeSize + childrenTotalSize;
}

/**
* @brief  A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of the required information from a Terminal Node, which has no descendants.
*/
template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline long CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::PreProcessorHelperTerminal(int depth, int uniqueHistoriesCnt) {

	/*std::cout << "Unique Terminal count: " << uniqueHistoriesCnt << "\n";*/

	long terminalNodeSize = sizeof(float);
	long totalTerminalNodesSize = terminalNodeSize * uniqueHistoriesCnt;
	//Update SizeAtDepth for future tree construction.
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(totalTerminalNodesSize);
	}
	else {
		mSizeAtDepth->at(depth) += totalTerminalNodesSize;
	}
	/*std::cout << "Terminal size : " << terminalNodeSize << "\n";*/
	return totalTerminalNodesSize;
}

/**
* @brief  A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of current Chance Node and all of its descendants in bytes.
*/
template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline long CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::PreProcessorHelper(ChanceNode chanceNode, int depth, int uniqueHistoriesCnt) {

	//Ensure mSizeAtDepth is large enough to store value at current depth
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(0);
	}

	ChanceNodeChildren* pChanceNodeChildren = mChildrenFromChanceFunc(chanceNode, mpGameInfo);

	/*
	Total size for float vector of probability of reaching each child node.
	*/
	long probabilityCnt = pChanceNodeChildren->GameSize() + pChanceNodeChildren->TerminalSize();
	long probabilitySize = probabilityCnt * (long) sizeof(float);

	/*
	sizeof(uint8_t) * 2:	required for storage of :
					- number of game children for tree traversal (1)
					- number of terminal children for tree traversal (1).

	sizeof(byte*)	: required for storage of:
					- starting offset for children.
	*/
	int nodeSize = probabilitySize + ( 2 * sizeof(uint8_t) ) + sizeof(long);;



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


	//Update SizeAtDepth for future tree construction.
	mSizeAtDepth->at(depth) += nodeSize;

	//Free memory on heap
	delete pChanceNodeChildren;

	/*std::cout << "Chance Node at depth " << depth << " size: "<< nodeSize << "\n";*/
	return nodeSize + childrenTotalSize;
}


/**
* @brief Preprocess GameTree before construction
* @return Size of entire game tree in bytes
*/
template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline long CFRGameTree<GameState, ChanceNode, Action, GameInfo>
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

template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::TreeConstructorHelper(GameState gameState, Strategy strategy, int depth, GameHistoryTreeNode* pHistoryTreeNode) {
	
	GameNodeChildren* pGameNodeChildren = mChildrenFromGameFunc(gameState, strategy, mpGameInfo);

	//Obtain offset using cumulativeOffsetAtDepth as part of Depth First Search tree construction.
	long offset = mCumulativeOffsetAtDepth->at(depth);
	long initialOffset = offset;

	int numActions = strategy.size();
	int numGameChildren = pGameNodeChildren->GameSize();
	int numChanceChildren = pGameNodeChildren->ChanceSize();
	int numTerminalChildren = pGameNodeChildren->TerminalSize();
	int totalNumChildren = numGameChildren + numChanceChildren + numTerminalChildren;
	
	//Set uint_8 for determing player who has action (1 : player 1 ; -1 : player 2)
	if (mPlayerOneHasAction(gameState, mpGameInfo)) {
		mGameTree[offset++] = (int8_t) 1;
	}
	else {
		mGameTree[offset++] = (int8_t) -1;
	}

	//Stores number of actions
	mGameTree[offset++] = (uint8_t) numActions;

	//Stores number of Game Node children
	mGameTree[offset++] = (uint8_t) numGameChildren;

	//Stores number of Chance Node children
	mGameTree[offset++] = (uint8_t) numChanceChildren;

	/*Stores number of Terminal Node children*/
	mGameTree[offset++] = (uint8_t) numTerminalChildren;

	//Stores starting offset of children.
	mGameTree[offset] = mCumulativeOffsetAtDepth->at(depth + 1);
	offset += sizeof(long);
		
	//Rock Paper Scissors Test.
	/*float zeroProb = 0.0;
	int totalActionsSize = numActions * sizeof(float);
	for (long iFloatOffset = offset; iFloatOffset < offset + totalActionsSize; iFloatOffset += sizeof(float)) {
		SetFloatAtBytePtr(mGameTree + iFloatOffset, zeroProb);
	}
	SetFloatAtBytePtr(mGameTree + offset, 1.0);*/

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

	//Offsets for recording many Actions -> Single Child relationship.
	long numActionsPerChildPos = offset;
	long actionPerChildPos = numActionsPerChildPos + ( totalNumChildren  * sizeof(uint8_t));
		
	
	using ChildGameNodes = GameNodeChildrenGamesNodes<GameState, Action>;
	using ChildChanceNodes = GameNodeChildrenChanceNodes<ChanceNode, Action>;
	using ChildTerminalNodes = GameNodeChildrenTerminalNodes<Action>;
	using ActionsToChild = std::vector<Action*>;

	/*Iterate over all children that are gamestates and number of actions per child.*/
	ChildGameNodes* childGameNodes = pGameNodeChildren->GetChildGameNodes();

	using ChildGameNode = GameNodeChildGameNode<GameState, Action>;

	typename std::vector<ChildGameNode*>::iterator iChildGameNode;
	typename std::vector<ChildGameNode*>::iterator iChildGameNodeEnd;
	iChildGameNode = childGameNodes->IterBegin();
	iChildGameNodeEnd = childGameNodes->IterEnd();

	std::vector<std::vector<int>> actionToChildArrs(numActions);
	uint8_t childIndex = 0;

	for (iChildGameNode; iChildGameNode < iChildGameNodeEnd; iChildGameNode++) {
		
		ChildGameNode* pChildGameNode = *iChildGameNode;
		GameState childGameState = pChildGameNode->GetGameState();
		Strategy childStrategy = pChildGameNode->GetStrategy();
		ActionsToChild parentActions = pChildGameNode->GetActionsToChild();

		
		mGameTree[numActionsPerChildPos] = parentActions.size();
		numActionsPerChildPos += sizeof(uint8_t);
		
	
		typename std::vector<Action*>::iterator iAction;
		typename std::vector<Action*>::iterator iActionEnd;
		iAction = parentActions.begin();
		iActionEnd = parentActions.end();

		for (iAction; iAction < iActionEnd; iAction++) {
			Action* pAction = *iAction;

			//Add index of action in strategy to ChildToActionArr
			auto it = std::find(strategy.begin(), strategy.end(), pAction);
			int actionIndex = it - strategy.begin();

			mGameTree[actionPerChildPos] = (uint8_t) actionIndex;
			actionPerChildPos += sizeof(uint8_t);
		}
		childIndex++;

		//Add to History Tree and recursively call on child.
		GameHistoryTreeNode *pChildTreeHistoryNode = pHistoryTreeNode->AddChild(childGameState, parentActions);
		TreeConstructorHelper(childGameState, childStrategy, depth + 1, pChildTreeHistoryNode);
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

	
		mGameTree[numActionsPerChildPos] = parentActions.size();
		numActionsPerChildPos += sizeof(uint8_t);
		

		typename std::vector<Action*>::iterator iAction;
		typename std::vector<Action*>::iterator iActionEnd;
		iAction = parentActions.begin();
		iActionEnd = parentActions.end();

		for (iAction; iAction < iActionEnd; iAction++) {
			Action* pAction = *iAction;

				//Add index of action in strategy to ChildToActionArr
				auto it = std::find(strategy.begin(), strategy.end(), pAction);
				int actionIndex = it - strategy.begin();

				mGameTree[actionPerChildPos] = (uint8_t) actionIndex;
				actionPerChildPos += sizeof(uint8_t);
		}
		childIndex++;

		//Add to History Tree and recursively call on child.
		GameHistoryTreeNode *pChildTreeHistoryNode = pHistoryTreeNode->AddChild(childChanceNode, parentActions);
		TreeConstructorHelper(childChanceNode, depth + 1, pChildTreeHistoryNode);
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

		mGameTree[numActionsPerChildPos] = parentActions.size();
		numActionsPerChildPos += sizeof(uint8_t);

		typename std::vector<Action*>::iterator iAction;
		typename std::vector<Action*>::iterator iActionEnd;
		iAction = parentActions.begin();
		iActionEnd = parentActions.end();

		for (iAction; iAction < iActionEnd; iAction++) {
			Action* pAction = *iAction;

			//Add index of action in strategy to ChildToActionArr
			auto it = std::find(strategy.begin(), strategy.end(), pAction);
			int actionIndex = it - strategy.begin();

			mGameTree[actionPerChildPos] = (uint8_t) actionIndex;
			actionPerChildPos += sizeof(uint8_t);
		}
		childIndex++;

		//Add to History Tree and recursively call on child to get number of terminal nodes.
		GameHistoryTreeNode *pChildTreeHistoryNode = pHistoryTreeNode->AddChild(parentActions);
		TreeConstructorHelperTerminal(depth + 1, pChildTreeHistoryNode);
	}

	offset = actionPerChildPos;

	//Update cumulativeOffsetAtDepth for Depth First Search tree construction.
	long offsetDiff = offset - initialOffset;
	mCumulativeOffsetAtDepth->at(depth) += offsetDiff;
	
	//Free memory on the heap
	delete pGameNodeChildren;
	delete pHistoryTreeNode;
	
	return;
}

/**
 * @brief Sets current TerminalNode and metadata in gameTree.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param pCurrNode
 * @param depth
 */
template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::TreeConstructorHelperTerminal(int depth, GameHistoryTreeNode* pHistoryTreeNode) {
	
	long offset = mCumulativeOffsetAtDepth->at(depth);
	int initialOffset = offset;

	//Find all unique histories from root to terminal node.
	using AllHistories = HistoryArray<GameState, ChanceNode, Action>;
	AllHistories *pHistories = GameHistoryTree::GetAllHistoriesToNode(pHistoryTreeNode);

	for (int iHistory = 0; iHistory < pHistories->NumHistories(); iHistory++) {
		
		History historyArr = pHistories->GetPath(iHistory);

		//Find utility value at terminal node using history.
		float utilityVal = mUtilityFunc(historyArr, mpGameInfo);

		//Stores utility for terminal node for player 1
		SetFloatAtBytePtr(mGameTree + offset, utilityVal);
		offset += sizeof(float);
	}

	//Update Cumulative Offset At Depth for Depth First Search Tree Construction.
	long offsetDiff = offset - initialOffset;
	mCumulativeOffsetAtDepth->at(depth) += offsetDiff;

	//Delete Histories Array and History Tree Node to make space on heap.
	delete pHistories;
	delete pHistoryTreeNode;
	
}

/**
 * @brief Sets current ChanceNode and metadata in gameTree and recursively sets its children.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param pCurrNode
 * @param depth
 */
template< typename GameState, typename ChanceNode, typename Action, typename GameInfo >
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::TreeConstructorHelper(ChanceNode chanceNode, int depth, GameHistoryTreeNode* pHistoryTreeNode) {
	
	long offset = mCumulativeOffsetAtDepth->at(depth);
	int initialOffset = offset;

	ChanceNodeChildren* pChildrenNodes = mChildrenFromChanceFunc(chanceNode, mpGameInfo);

	//Stores number of non terminal children                                                                                                                                                                                                            
	int numGameChildren = pChildrenNodes->GameSize();
	mGameTree[offset++] = (uint8_t) numGameChildren;

	/*Stores number of terminal children*/
	int numTerminalChildren = pChildrenNodes->TerminalSize();
	mGameTree[offset++] = (uint8_t) numTerminalChildren;

	//Stores starting offset for children
	mGameTree[offset] = mCumulativeOffsetAtDepth->at(depth + 1);
	offset += (long) sizeof(long);

	
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

		//Set probability of reaching child in game tree.
		SetFloatAtBytePtr(mGameTree + offset, probToChild);
		offset += sizeof(float);

		//Add History node to history for next level of Game Tree
		GameHistoryTreeNode* pChildHistoryTreeNode = pHistoryTreeNode->AddChild(childGameState);

		TreeConstructorHelper(childGameState, childStrategy, depth + 1, pChildHistoryTreeNode);
	}

	//Iterate over all children that are terminal nodes.
	ChildTerminalNodes* childTerminalNodes = pChildrenNodes->GetChildTerminalNodes();

	using ChildTerminalNode = ChanceNodeChildTerminalNode;

	typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNode;
	typename std::vector<ChildTerminalNode*>::iterator iChildTerminalNodeEnd;
	iChildTerminalNode = childTerminalNodes->IterBegin();
	iChildTerminalNodeEnd = childTerminalNodes->IterEnd();

	long numTerminalChildrenPos = initialOffset + 4;

	for (iChildTerminalNode; iChildTerminalNode < iChildTerminalNodeEnd; iChildTerminalNode++) {
		ChildTerminalNode* pChildTerminalNode = *iChildTerminalNode;
		float probToChild = pChildTerminalNode->GetProbToChild();
		
		SetFloatAtBytePtr(mGameTree + offset, probToChild);
		offset += sizeof(float);

		//Add History node to history for next level of Game Tree
		GameHistoryTreeNode* pChildHistoryTreeNode = pHistoryTreeNode->AddChild();
		
		TreeConstructorHelperTerminal(depth + 1, pChildHistoryTreeNode);
	}
	//Free space on heap
	delete pChildrenNodes;
	delete pHistoryTreeNode;

	//Update Cumulative Offset At Depth for Depth First Search Tree Construction
	long offsetDiff = offset - initialOffset;
	mCumulativeOffsetAtDepth->at(depth) += offsetDiff;
	
	return;
}


/**
 * @brief Calls Tree Constructor Helper on starting Chance node (the root of the tree).
 *		  Sets values for CumulativeOffsetAtDepth that continously updates throughout
 *		  the traversal of the tree.
		  This creates a tree structure that is organized by the depth of nodes.
 */
template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::ConstructTree() {
	int treeSize = PreProcessor();
	mGameTree = new byte[treeSize];
	GameHistoryTreeNode *pStartingHistory = new GameHistoryTreeNode(mStartingChanceNode, nullptr);

	TreeConstructorHelper(mStartingChanceNode, 0, pStartingHistory);
}

/**
 * @brief Calculates utility for gamenode at pGameNodePos by recursively calling on its children.
 * @param treeGameNode 
 * @param reachProbPlayerOne 
 * @param reachProbPlayerTwo 
 * @param terminalIndex 
 * @return Pair where first element = utility of gamenode, second element = position of next node in tree.
 */

template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
float CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::CFRHelper(TreeGameNode* treeGameNode, float reachProbPlayerOne, float reachProbPlayerTwo, int terminalIndex) {

	//Variable for determing which player has action at the current node.
	int playerTurn = treeGameNode->mPlayerToAct;

	//Get All Non Terminal Children
	TreeNodeChildren allChildren = treeGameNode->GetChildren();

	//Initialize Child index for Parent action to child retrieval.
	int childIndex = 0;

	//Initialize value of current game node.
	float val = 0;

	//Initialize vector for temporarily storing child of action utilities.
	std::vector<float> actionUtilities(treeGameNode->mNumActions, 0);

	//Start offset for terminal children to be updated by child game and chance nodes.
	long terminalChildOffset = treeGameNode->mpChildStartOffset;

	//Recursively call CFR Helper on all children Tree Game Nodes.
	std::vector<TreeGameNode> gameNodeChildren = allChildren.GetChildrenGameNodes();
	for (TreeGameNode gameNodeChild : gameNodeChildren) {
		
		//Get all actions of current tree node that lead to current chld node.
		std::vector<uint8_t> actionIndices = treeGameNode->GetActionIndicesForChild(childIndex);

		//Update terminal index using number of actions to current child.
		int childTerminalIndexStart = terminalIndex * actionIndices.size();
		
		int actionIndex = 0;
		for (int iAction : actionIndices) {

			//Calculate terminal index based on current tree path.
			int newTerminalIndex = childTerminalIndexStart + actionIndex;

			//Calculate Reach Probabilities for each player.
			float childReachProbOne = 1.0;
			float childReachProbTwo = 1.0;
			float currStratProb = treeGameNode->GetCurrStratProb(iAction);
			if (playerTurn == 1) {
				childReachProbOne = reachProbPlayerOne * currStratProb;
			}
			/*If it isn't player 1's turn, it is player 2's turn*/
			else {
				childReachProbTwo = reachProbPlayerTwo * currStratProb;
			}
			//Recursively call on child with history that includes iAction to get utility.
			float actionUtility = CFRHelper(&gameNodeChild, childReachProbOne, childReachProbTwo, newTerminalIndex);
			actionUtilities.at(iAction) += actionUtility;
			
			//Update value of current game node.
			val += currStratProb * actionUtility;

			actionIndex++;
		}
		//Update terminal child start offset.
		terminalChildOffset = gameNodeChild.mpNextNodePos;

		//Update child index
		childIndex++;
	}
	/*Recursively call CFR Helper on all children Tree Chance Nodes.*/
	std::vector<TreeChanceNode> chanceNodeChildren = allChildren.GetChildrenChanceNodes();
	for (TreeChanceNode chanceNodeChild : chanceNodeChildren) {
	
		std::vector<uint8_t> actionIndices = treeGameNode->GetActionIndicesForChild(childIndex);
		int childTerminalIndexStart = terminalIndex * actionIndices.size();
		int actionIndex = 0;
		for (int iAction : actionIndices) {
			int newTerminalIndex = childTerminalIndexStart + actionIndex;
			float childReachProbOne = 1.0;
			float childReachProbTwo = 1.0;
			float currStratProb = treeGameNode->GetCurrStratProb(iAction);

			if (playerTurn == 1) { childReachProbOne = reachProbPlayerOne * currStratProb; }
			else { childReachProbTwo = reachProbPlayerTwo * currStratProb; }

			//Recursively call on child with history that includes iAction to get utility.
			float actionUtility = CFRHelper(&chanceNodeChild, childReachProbOne, childReachProbTwo, newTerminalIndex);
			actionUtilities.at(iAction) += actionUtility;

			//Update value of current game node.
			val += currStratProb * actionUtility;

			actionIndex++;
		}
		//Update terminal child start offset.
		terminalChildOffset = chanceNodeChild.mpNextNodePos;

		//Update child index
		childIndex++;
	}
	//If node has any terminal children, find them and get utilities.
	int numUserTerminalNodes = treeGameNode->mNumTerminalChildren;
	long currTerminalChildStart = terminalChildOffset;
	int terminalNodeSize = TreeTerminalNode::NodeSize();

	for (int iTerminal = 0; iTerminal < numUserTerminalNodes; iTerminal++) {
		std::vector<uint8_t> actionIndices = treeGameNode->GetActionIndicesForChild(childIndex);
		int childTerminalIndexStart = terminalIndex * actionIndices.size();
			
		int actionIndex = 0;
		for (int iAction : actionIndices) {
			int newTerminalIndex = childTerminalIndexStart + actionIndex;

			float currStratProb = treeGameNode->GetCurrStratProb(iAction);

			//Find Terminal utility using newTerminalIndex and child terminal start pos.
			long currTerminalPos = currTerminalChildStart + (terminalNodeSize * newTerminalIndex);
			TreeTerminalNode terminalNode = TreeTerminalNode(mGameTree, currTerminalPos);

			//Get utility from terminal node.
			float actionUtility = terminalNode.mUtilityVal;
			actionUtilities.at(iAction) += actionUtility;

			//Update value of current game node.
			val += currStratProb * actionUtility;

			actionIndex++;
		}

		childIndex++;
		currTerminalChildStart += terminalNodeSize * actionIndices.size();
	}	
	float cfrReach;
	float reach;
	if (playerTurn == 1) {
		cfrReach = reachProbPlayerTwo;
		reach = reachProbPlayerOne;
	}
	/*If it isn't player 1's turn, it is player 2's turn*/
	else {
		cfrReach = reachProbPlayerOne;
		reach = reachProbPlayerTwo;
	}
	for (int iAction = 0; iAction < treeGameNode->mNumActions; iAction++) {
		float regretMultiplier = (float) playerTurn;
		float actionUtility = actionUtilities.at(iAction);
		float currStratProb = treeGameNode->GetCurrStratProb(iAction);
		float actionCfrRegret = regretMultiplier * cfrReach * ( actionUtility - val );
		treeGameNode->AddCumRegret(actionCfrRegret, iAction);
		treeGameNode->AddCumStratProb(reach * currStratProb, iAction);
	}
	return val;
}

template< typename GameState, typename ChanceNode, typename Action, typename GameInfo >
float CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::CFRHelper(TreeChanceNode* chanceGameNode, float reachProbPlayerOne, float reachProbPlayerTwo, int terminalIndex) {

	int val = 0;
	TreeNodeChildren treeChildren = chanceGameNode->GetChildren();
	std::vector<TreeGameNode> gameNodeChildren = treeChildren.GetChildrenGameNodes();

	int childIndex = 0;
	long terminalNodeStart = chanceGameNode->mpNextNodePos;
	//Iterate through all child gamenodes to find node value.
	for (TreeGameNode treeGameNode : gameNodeChildren) {
		float childUtil = CFRHelper(&treeGameNode, reachProbPlayerOne, reachProbPlayerTwo, terminalIndex);
		float childReachProb = chanceGameNode->GetChildReachProb(childIndex);
		val += childReachProb * childUtil;

		terminalNodeStart = treeGameNode.mpNextNodePos;
		childIndex++;
	}
	//Iterate through all terminal nodes to find utility.
	int numTerminalChildren = chanceGameNode->mNumTerminalChildren;
	int terminalNodeSize = TreeTerminalNode::NodeSize();
	int iTerminalEnd = terminalNodeStart + (numTerminalChildren *  terminalNodeSize);
	int iTerminal = terminalNodeStart;
	for (iTerminal; iTerminal < iTerminalEnd; iTerminal += terminalNodeSize) {

		byte* pUtility = (byte*) (mGameTree + iTerminal);
		float childUtil = GetFloatFromBytePtr(pUtility);

		float childReachProb = chanceGameNode->GetChildReachProb(childIndex);
		val += childReachProb * childUtil;
		childIndex++;
	}
	return val;
}

template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::UpdateStratProbs(TreeGameNode *treeGameNode) {
	
	int numActions = treeGameNode->mNumActions;
	float regretSum = 0;
	for (int iAction = 0; iAction < numActions; iAction++) {
		float actionCumRegret = treeGameNode->GetCumRegret(iAction);
		if (actionCumRegret > 0) {
			regretSum += actionCumRegret;
		}
	}
	for (int iAction = 0; iAction < numActions; iAction++) {
		float cumRegret = treeGameNode->GetCumRegret(iAction);
		float updatedProb = regretSum > 0 ? (std::max(cumRegret, 0.0f) / regretSum) : (1.0f / (float) numActions);
		treeGameNode->SetCurrStratProb(updatedProb, iAction);
	}
	//Recursively call on all non terminal children.
	TreeNodeChildren children = treeGameNode->GetChildren();
	std::vector<TreeGameNode> gameNodeChildren = children.GetChildrenGameNodes();
	for (TreeGameNode childGameNode : gameNodeChildren) {
		UpdateStratProbs(&childGameNode);
	}
	std::vector<TreeChanceNode> chanceNodeChildren = children.GetChildrenChanceNodes();
	for (TreeChanceNode childChanceNode : chanceNodeChildren) {
		UpdateStratProbs(&childChanceNode);
	}
}

template< typename GameState, typename ChanceNode, typename Action, typename GameInfo >
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::UpdateStratProbs(TreeChanceNode *treeChanceNode) {
	TreeNodeChildren children = treeChanceNode->GetChildren();
	//Recursively call on all game node children.
	std::vector<TreeGameNode> gameNodeChildren = children.GetChildrenGameNodes();
	for (TreeGameNode childGameNode : gameNodeChildren) {
		UpdateStratProbs(&childGameNode);
	}
}



template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::CFR(int iterations) {
	
	TreeChanceNode rootNode = TreeChanceNode(mGameTree, 0);
	for (int iCFR = 0; iCFR < iterations; iCFR++) {
		CFRHelper(&rootNode, 1.0, 1.0, 0);		
		UpdateStratProbs(&rootNode);
		
	}
	UpdateNashStrategy(&rootNode, iterations);
	return;
}

template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::CFRAccuracy(float accuracy) {

	TreeChanceNode rootNode = TreeChanceNode(mGameTree, 0);
	float prevGameValue = CFRHelper(&rootNode, 1.0, 1.0, 0);
	UpdateStratProbs(&rootNode);
	int iCFR = 0;
	while (true) {
		int currValue = CFRHelper(&rootNode, 1.0, 1.0, 0);
		UpdateStratProbs(&rootNode);
		iCFR++;
		float dEVpct = std::abs(currValue - prevGameValue) / (float) 100;
		//If desired accuracy is reached, stop iterating.
		if (dEVpct <= accuracy ){ 
			break; 
		}
		prevGameValue = currValue;
	}
	UpdateNashStrategy(&rootNode, iCFR);
	return;
}

template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::UpdateNashStrategy(TreeGameNode* treeGameNode, int numIterations) {
	
	int numActions = treeGameNode->mNumActions;
	float unnormalizedTotal = 0;
	for (int iAction = 0; iAction < numActions; iAction++) {
		float cumStratProb = treeGameNode->GetCumStratProb(iAction);
		float avgStratProb = cumStratProb / (float) numIterations;
		treeGameNode->SetCurrStratProb(avgStratProb, iAction);
		unnormalizedTotal += avgStratProb;
	}
	float normalizeMultiplier = 1.0f / unnormalizedTotal;
	for (int iAction = 0; iAction < numActions; iAction++) {
		float unNormalizedStratProb = treeGameNode->GetCurrStratProb(iAction);
		float normalizedStratProb = unNormalizedStratProb * normalizeMultiplier;
		treeGameNode->SetCurrStratProb(normalizedStratProb, iAction);
	}
	//Recursively call on all non terminal children.
	TreeNodeChildren children = treeGameNode->GetChildren();
	std::vector<TreeGameNode> gameNodeChildren = children.GetChildrenGameNodes();
	for (TreeGameNode childGameNode : gameNodeChildren) {
		UpdateNashStrategy(&childGameNode, numIterations);
	}
	std::vector<TreeChanceNode> chanceNodeChildren = children.GetChildrenChanceNodes();
	for (TreeChanceNode childChanceNode : chanceNodeChildren) {
		UpdateNashStrategy(&childChanceNode, numIterations);
	}
	
	
}

template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>
::UpdateNashStrategy(TreeChanceNode* treeChanceNode, int numIterations) {
	TreeNodeChildren children = treeChanceNode->GetChildren();
	//Recursively call on all game node children.
	std::vector<TreeGameNode> gameNodeChildren = children.GetChildrenGameNodes();
	for (TreeGameNode childGameNode : gameNodeChildren) {
		UpdateNashStrategy(&childGameNode, numIterations);
	}
}


template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>::PrintGameTree() {
	TreeChanceNode root = TreeChanceNode(mGameTree, 0);
	PrintTreeHelper(&root);

}

template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>::PrintTreeHelper(TreeGameNode* treeGameNode) {

	std::cout << *treeGameNode;
	//Recursively call on all non terminal children.
	TreeNodeChildren children = treeGameNode->GetChildren();
	std::vector<TreeGameNode> gameNodeChildren = children.GetChildrenGameNodes();
	for (TreeGameNode childGameNode : gameNodeChildren) {
		PrintTreeHelper(&childGameNode);
	}
	std::vector<TreeChanceNode> chanceNodeChildren = children.GetChildrenChanceNodes();
	for (TreeChanceNode childChanceNode : chanceNodeChildren) {
		PrintTreeHelper(&childChanceNode);
	}
}


template<typename GameState, typename ChanceNode, typename Action, typename GameInfo>
inline void CFRGameTree<GameState, ChanceNode, Action, GameInfo>::PrintTreeHelper(TreeChanceNode* treeChanceNode) {
	TreeNodeChildren children = treeChanceNode->GetChildren();
	//Recursively call on all game node children.
	std::vector<TreeGameNode> gameNodeChildren = children.GetChildrenGameNodes();
	for (TreeGameNode childGameNode : gameNodeChildren) {
		PrintTreeHelper(&childGameNode);
	}
}








