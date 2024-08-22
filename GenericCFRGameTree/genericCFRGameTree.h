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

template <
	typename Action,
	typename CommonGameState,
	typename GameState,
	typename TerminalNode,
	typename ChanceNode
>
class CFRGameTree {

public:


	/*LIBRARY DEFINED TYPES*/
	typedef std::vector<Action*> StrategyProfile;
	typedef std::vector<float> Probabilities;

	typedef std::tuple<StrategyProfile*, Probabilities*> Strategy;
	typedef std::tuple<GameState*, StrategyProfile*> GameNode;
	typedef std::tuple<GameNode*, float> GameNodeWithProb;
	typedef std::tuple<TerminalNode*, float> TerminalNodeWithProb;

	typedef std::vector<StrategyProfile*> StrategyProfileList;

	typedef std::vector<GameNode*> GameNodeList;
	typedef std::vector<TerminalNode*> TerminalNodeList;
	typedef std::vector<ChanceNode*> ChanceNodeList;

	typedef std::vector<GameNodeWithProb*> GameNodeListFromChance;
	typedef std::vector<TerminalNodeWithProb*> TerminalNodeListFromChance;

	typedef std::tuple<GameNode*, TerminalNode*, ChanceNode*> ChildNode;
	typedef std::tuple<GameNodeListFromChance*, TerminalNodeListFromChance*> ChildrenFromChance;


	CFRGameTree() {

		mCommonState = nullptr;
		mChildNodeFunc = nullptr;
		mChildrenFromChanceFunc = nullptr;
		mStrategyProfileList = nullptr;
		mStartingChanceNode = nullptr;

		mPlayerOneHasAction = nullptr;

		mGameTree = nullptr;
		mSizeAtDepth = nullptr;
		mCumulativeOffsetAtDepth = nullptr;
		mUtilityFunc = nullptr;	
	}

	CFRGameTree(
		CommonGameState* commonState,
		std::tuple<GameNode*, TerminalNode*, ChanceNode*>* ( *childNodeFromGameNode )( GameState*, Action*, CommonGameState*, StrategyProfileList* ),
		ChildrenFromChance* ( *childNodeFromChanceNode ) ( ChanceNode*, CommonGameState*, StrategyProfileList* ),
		bool ( *playerOneHasAction ) ( GameState* ),
		ChanceNode* startingChanceNode,
		StrategyProfileList* allStrategies,
		float ( *utilityFromTerminal )( TerminalNode* )
	) {
		mCommonState = commonState;
		mChildNodeFunc = childNodeFromGameNode;
		mChildrenFromChanceFunc = childNodeFromChanceNode;
		mPlayerOneHasAction = playerOneHasAction;
		mStrategyProfileList = allStrategies;
		mStartingChanceNode = startingChanceNode;
		mUtilityFunc = utilityFromTerminal;

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

	/*Pointer to state that is constant throughout game tree*/
	CommonGameState *mCommonState;

	/*Pointer to root of Game Tree (Chance Node)*/
	ChanceNode *mStartingChanceNode;
	
	/*Pointer to list of all strategies in game tree*/
	StrategyProfileList *mStrategyProfileList;	
	
	/*Pointer to list of size in bytes of each level of Game tree */
	std::vector<long> *mSizeAtDepth;			
	
	/*Pointer to list of cumulative offsets of each level of Game Tree*/
	std::vector<long> *mCumulativeOffsetAtDepth;

	/**
	 * @brief User defined function returns Child Node from
			  a given GameState, Action, and Common Game State
	 */
	ChildNode* (*mChildNodeFunc)( GameState*, Action*, CommonGameState*, StrategyProfileList* );

	/**
	 * @brief User defined function returns 2-tuple of list pointers.
	 *		  1st elem is a list of Child Game Nodes with probability from the Chance Node.
	 *		  2nd elem is a list of Child Terminal Nodes with probability from the Chance Node.
	 */
	ChildrenFromChance* (*mChildrenFromChanceFunc) ( ChanceNode*, CommonGameState*, StrategyProfileList*);

	/**
	 * @brief User defined function that gets player who has action for a given gamestate
	 * @param pCurrGameState 
	 * @return true if player 1 has action, false if player 2 has action.
	 */
	bool (* mPlayerOneHasAction) (GameState* pCurrGameState);
	
	/**
	 * @brief User defined function that gets utility value from given Terminal Node.
	 * @param pCurrTerminalNode
	 * @return (float) value of utility of pCurrTerminalNode.
	 */
	float ( *mUtilityFunc ) ( TerminalNode* pCurrTerminalNode );


	

	/*LIBRARY DEFINED FUNCTIONS*/
	/**
	 * @brief Overloaded Helper function to find the type of a child node.
	 * @param  ChildNode*
	 * @return Returns true if the ChildNode matches the Node type specified by the function name.
	 *		   Returns false otherwise.
	 */
	static bool IsChildGameNode(ChildNode*);
	static bool IsChildTerminal(ChildNode*);
	static bool IsChildChance(ChildNode*);

	/**
	 * @brief Overloaded Helper functions for pre - processing the game tree.
	 * @param Node*
	 * @return Returns the (long) size of a serialized node and all of its descendants.
	 */
	long PreProcessorHelper(GameNode*, int depth);
	long PreProcessorHelper(TerminalNode*, int depth);
	long PreProcessorHelper(ChanceNode*, int depth);

	/**
	 * @brief Overloaded Helper functions for constructing the game tree
	 *		  Sets bytes of allocated gameTree to appropiate representation.
	 * @param  Node*
	 * @param  depth
	 */
	void TreeConstructorHelper(GameNode* node, int depth);
	void TreeConstructorHelper(TerminalNode* node, int depth);
	void TreeConstructorHelper(ChanceNode* node, int depth);
	

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
* @brief A helper function to find the type of a child node.
* @param pChildNode
* @return true if child node is a Game Node, false otherwise
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::IsChildGameNode(CFRGameTree::ChildNode* pChildNode) {
	return std::get<0>(*pChildNode) != nullptr;
}

/**
* @brief A helper function to find the type of a child node.
* @param pChildNode
* @return true if child node is a Terminal Node, false otherwise
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::IsChildTerminal(CFRGameTree::ChildNode* pChildNode) {
	return std::get<1>(*pChildNode) != nullptr;
}

/**
* @brief A helper function to find the type of a child node.
* @param pChildNode
* @return true if child node is a Chance Node, false otherwise
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::IsChildChance(CFRGameTree::ChildNode* pChildNode) {
	return std::get<2>(*pChildNode) != nullptr;
}



/**
* @brief A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of the required information from a GameNode and all of its descendants in bytes.
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::PreProcessorHelper(CFRGameTree::GameNode* pCurrNode, int depth) {
	
	//Ensure sizeAtDepth is large enough to store value at current depth
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(0);
	}
	GameState* pCurrGameState = std::get<GameState*>(*pCurrNode);
	CFRGameTree::StrategyProfile* pCurrStrategy = std::get<CFRGameTree::StrategyProfile*>(*pCurrNode);

	
	/*
	Strategy size = number of actions * sizeof(float) * 3 
	Multiplying by 3 is the result of needing to store:
		- Current Strategy
		- Cumulative sum of strategies
		- Immediate counterfactual regret for each action.
	*/
	int strategySize = 3 * sizeof(float) * pCurrStrategy->size();

	/*
	sizeof(uint8_t) * 3:	required for storage of :
					- node identifier "c" for cfr updates (1)
					- number of children / actions for tree traversal / cfr updates (1)
					- boolean that is true when player 1 has action / false otherwise.
	
	sizeof(byte*)	: required for storage of:
					- starting offset for children.
	*/
	int nodeSize = strategySize + (3 * sizeof(uint8_t)) + sizeof(byte*);

	//Recursively find the size of children to add to total sum.
	int childrenTotalSize = 0;
	for (auto iAction = pCurrStrategy->begin(); iAction != pCurrStrategy->end(); iAction++) {
		CFRGameTree::ChildNode* pChildNode = mChildNodeFunc(pCurrGameState, *iAction, mCommonState, mStrategyProfileList);
		if (IsChildGameNode(pChildNode)) {
			childrenTotalSize += PreProcessorHelper(std::get<0>(*pChildNode), depth + 1);

		}
		if (IsChildTerminal(pChildNode)) {
			childrenTotalSize += PreProcessorHelper(std::get<1>(*pChildNode), depth + 1);
		}
		if (IsChildChance(pChildNode)) {
			childrenTotalSize += PreProcessorHelper(std::get<2>(*pChildNode), depth + 1);
		}
		//Clean up allocated memory on heap
		delete pChildNode;
	}
	//Update SizeAtDepth for future tree construction.
	mSizeAtDepth->at(depth) += nodeSize;

	//Clean up allocated memory on heap
	delete pCurrGameState;

	return nodeSize + childrenTotalSize;
}

/**
* @brief  A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of the required information from a Terminal Node, which has no descendants.
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::PreProcessorHelper(TerminalNode* pCurrNode, int depth){
	
	long terminalNodeSize = sizeof(float) + sizeof(byte);
	//Update SizeAtDepth for future tree construction.
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(terminalNodeSize);
	}
	else {
		mSizeAtDepth->at(depth) += terminalNodeSize;
	}
	return terminalNodeSize;
}

/**
* @brief  A helper function for pre - processing the game tree.
* @param pCurrNode
* @return The size of current Chance Node and all of its descendants in bytes.
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
	::PreProcessorHelper(ChanceNode* pCurrNode, int depth) {
	
	//Ensure mSizeAtDepth is large enough to store value at current depth
	if (mSizeAtDepth->size() <= depth) {
		mSizeAtDepth->push_back(0);
	}

	CFRGameTree::ChildrenFromChance* pChildrenNodes = mChildrenFromChanceFunc(pCurrNode, mCommonState, mStrategyProfileList);

	CFRGameTree::GameNodeListFromChance* pChildrenGameNodes = std::get<CFRGameTree::GameNodeListFromChance*>(*pChildrenNodes);
	CFRGameTree::TerminalNodeListFromChance* pChildrenTerminalNodes = std::get<CFRGameTree::TerminalNodeListFromChance*>(*pChildrenNodes);

	/*
	Total size for float vector of probability of reaching each child node.
	*/
	long probabilityCnt = pChildrenGameNodes->size() + pChildrenTerminalNodes->size();
	long probabilitySize = probabilityCnt * (long) sizeof(float);

	/*
	sizeof(uint8_t) * 2:	required for storage of :
					- node identifier "c" for deserialization (1)
					- number of children for tree traversal (1)
	 
	sizeof(byte*)	: required for storage of:
					- starting offset for children.
	*/
	int nodeSize = probabilitySize + ( 2 * sizeof(uint8_t) ) + sizeof(long);;

	//Recursively find the size of children to add to total sum.
	long childrenTotalSize = 0;
	for (auto iGameNode = pChildrenGameNodes->begin(); iGameNode != pChildrenGameNodes->end(); iGameNode++) {
		GameNode* pChildGameNode = std::get<GameNode*>(**iGameNode);
		childrenTotalSize += PreProcessorHelper(pChildGameNode, depth + 1);
		//Clean up allocated memory on heap
		delete pChildGameNode;
	}
	for (auto iTerminalNode = pChildrenTerminalNodes->begin(); iTerminalNode != pChildrenTerminalNodes->end(); iTerminalNode++) {
		TerminalNode* pChildTerminalNode = std::get<TerminalNode*>(**iTerminalNode);
		childrenTotalSize += PreProcessorHelper(pChildTerminalNode, depth + 1);
		//Clean up allocated memory on heap
		delete pChildTerminalNode;
	}
	//Update SizeAtDepth for future tree construction.
	mSizeAtDepth->at(depth) += nodeSize;

	//Clean up allocated memory on heap
	delete pChildrenNodes;
	delete pChildrenGameNodes;
	delete pChildrenTerminalNodes;

	return nodeSize + childrenTotalSize;
}


/**
* @brief Preprocess GameTree before construction
* @return Size of entire game tree in bytes
*/
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::PreProcessor() {
	//Initialize sizeAtDepth vector for future tree construction.
	mSizeAtDepth = new std::vector<long>();
	long treeSize = PreProcessorHelper(mStartingChanceNode, 0);

	/*Iterate through updated sizeAtDepth vector for cumulative depth offset.
	  This is essential for future tree construction via Depth First Search.
	*/
	long cumulativeDepthOffset = 0;
	mCumulativeOffsetAtDepth = new std::vector<long>(mSizeAtDepth->size(), 0);
	for (int iSizeAtDepth = 1; iSizeAtDepth < mSizeAtDepth->size(); iSizeAtDepth++) {
		long depthSize = mSizeAtDepth->at(iSizeAtDepth - 1);
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
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::TreeConstructorHelper(GameNode* pCurrNode, int depth) {
	
	GameState* pCurrGameState = std::get<GameState*>(*pCurrNode);
	CFRGameTree::StrategyProfile* pCurrStrategy = std::get<CFRGameTree::StrategyProfile*>(*pCurrNode);

	//Obtain offset using cumulativeOffsetAtDepth as part of Depth First Search tree construction.
	long offset = mCumulativeOffsetAtDepth->at(depth);
	long initialOffset = offset;

	//Set classifier "g" at offset to indicate that this node is a gamenode
	mGameTree[offset++] = 'g';

	//Set uint_8 for determing player who has action (1 : player 1 ; -1 : player 2)
	if (mPlayerOneHasAction(pCurrGameState)) {
		mGameTree[offset++] = (int8_t) 1;
	}
	else {
		mGameTree[offset++] = (int8_t) -1;
	}
	
	//Stores number of children / number of acitons
	int numActions = pCurrStrategy->size();
	mGameTree[offset++] = (uint8_t) numActions;

	//Stores floating point values for cumulative regret, strategy probabilities, and current regret.
	float uniform_prob = 1.0 / ((float) numActions);
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

	//Recursively call Tree Constructor Helper on each child at next depth.
	for (auto iAction = pCurrStrategy->begin(); iAction != pCurrStrategy->end(); iAction++) {
		CFRGameTree::ChildNode* pChildNode = mChildNodeFunc(pCurrGameState, *iAction, mCommonState, mStrategyProfileList);
		if (IsChildGameNode(pChildNode)) {
			GameNode* pChildGameNode = std::get<0>(*pChildNode);
			TreeConstructorHelper(pChildGameNode, depth + 1);
		}
		if (IsChildTerminal(pChildNode)) {
			TerminalNode* pChildTerminalNode = std::get<1>(*pChildNode);
			TreeConstructorHelper(pChildTerminalNode, depth + 1);
		}
		if (IsChildChance(pChildNode)) {
			ChanceNode* pChildChanceNode = std::get<2>(*pChildNode);
			TreeConstructorHelper(pChildChanceNode, depth + 1);
		}
		//Clean up allocated memory on heap
		delete pChildNode;
	}

	//Clean up allocated memory on heap
	delete pCurrGameState;
	
	return;
}

/**
 * @brief Sets current TerminalNode and metadata in gameTree.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param pCurrNode
 * @param depth
 */
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::TreeConstructorHelper(TerminalNode* pCurrNode, int depth) {
	long offset = mCumulativeOffsetAtDepth->at(depth);
	int initialOffset = offset;

	//Set classifier to "t" to identify that the node is a Terminal Node
	mGameTree[offset] = 't';
	offset++;

	float utilityVal = mUtilityFunc(pCurrNode);
	//Stores utility for terminal node for player 1 (multiply by -1 for player 2)
	SetFloatAtBytePtr(mGameTree + offset, utilityVal);
	offset += sizeof(float);

	//Update Cumulative Offset At Depth for Depth First Search Tree Construction.
	long offsetDiff = offset - initialOffset;
	mCumulativeOffsetAtDepth->at(depth) += offsetDiff;
	return;
}

/**
 * @brief Sets current ChanceNode and metadata in gameTree and recursively sets its children.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param pCurrNode
 * @param depth
 */
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::TreeConstructorHelper(ChanceNode* pCurrNode, int depth) {
	
	long offset = mCumulativeOffsetAtDepth->at(depth);
	int initialOffset = offset;

	//Set classifier to "c" to identify that the node is a Chance Node
	mGameTree[offset++] = 'c';

	CFRGameTree::ChildrenFromChance* pChildrenNodes = mChildrenFromChanceFunc(pCurrNode, mCommonState, mStrategyProfileList);
	CFRGameTree::GameNodeListFromChance* pChildrenGameNodes = std::get<CFRGameTree::GameNodeListFromChance*>(*pChildrenNodes);
	CFRGameTree::TerminalNodeListFromChance* pChildrenTerminalNodes = std::get<CFRGameTree::TerminalNodeListFromChance*>(*pChildrenNodes);

	//Stores number of children 
	int numChildren = pChildrenGameNodes->size() + pChildrenTerminalNodes->size();
	mGameTree[offset++] = (uint8_t) numChildren;

	//Stores starting offset for children
	mGameTree[offset] = mCumulativeOffsetAtDepth->at(depth + 1);
	offset += (long) sizeof(long);


	/*Recursively call Tree Constructor Helper on each child at next depth.
	  Store probability of reaching each child 
	*/
	for (auto iChildGameNode = pChildrenGameNodes->begin(); iChildGameNode != pChildrenGameNodes->end(); iChildGameNode++) {
		float childProb = std::get<float>(**iChildGameNode);
		SetFloatAtBytePtr(mGameTree + offset, childProb);
		offset += sizeof(float);
		GameNode* pChildGameNode = std::get<GameNode*>(**iChildGameNode);
		GameNode gameNodeTemp = *(pChildGameNode);
		TreeConstructorHelper(&(gameNodeTemp), depth + 1);
		//Clean up allocated memory on heap
		delete pChildGameNode;
	}
	for (auto iChildTerminalNode = pChildrenTerminalNodes->begin(); iChildTerminalNode != pChildrenTerminalNodes->end(); iChildTerminalNode++) {
		float childProb = std::get<float>(**iChildTerminalNode);
		SetFloatAtBytePtr(mGameTree + offset, childProb);
		offset += sizeof(float);
		TerminalNode* pChildTerminalNode = std::get<TerminalNode*>(**iChildTerminalNode);
		TreeConstructorHelper(pChildTerminalNode, depth + 1);
		//Clean up allocated memory on heap
		delete pChildTerminalNode;
	}

	//Update Cumulative Offset At Depth for Depth First Search Tree Construction
	long offsetDiff = offset - initialOffset;
	mCumulativeOffsetAtDepth->at(depth) += offsetDiff;

	//Clean up allocated memory on heap
	delete pChildrenNodes;
	delete pChildrenGameNodes;
	delete pChildrenTerminalNodes;

	return;
}


/**
 * @brief Calls Tree Constructor Helper on starting Chance node (the root of the tree).
 *		  Sets values for CumulativeOffsetAtDepth that continously updates throughout
 *		  the traversal of the tree.
		  This creates a tree structure that is organized by the depth of nodes.
 */
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::ConstructTree() {
	int treeSize = PreProcessor();
	mGameTree = new byte[treeSize];
	TreeConstructorHelper(mStartingChanceNode, 0);
}


template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline std::pair<float, long> CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
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
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline std::pair<float, long> CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::CFRGameNodeHelper(TreeGameNode* treeGameNode, float reachProbPlayerOne, float reachProbPlayerTwo) {
		
	long pChildPos = treeGameNode->mpChildStartOffset;
	std::vector<float> childrenUtilities(treeGameNode->mNumChildrenOrActions);
	float val = 0;
	int playerTurn = treeGameNode->mPlayerToAct;
	for (int iChild = 0; iChild < treeGameNode->mNumChildrenOrActions; iChild++) {
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
	for (int iChild = 0; iChild < treeGameNode->mNumChildrenOrActions; iChild++) {
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
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline std::pair<float, long> CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
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

template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
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

template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::UpdateStratGameNode(TreeGameNode *treeGameNode) {
	
	int numChildren = treeGameNode->mNumChildrenOrActions;
	int numActions = numChildren;
	float regretSum = 0;
	for (int iAction = 0; iAction < numChildren; iAction++) {
		float actionCumRegret = treeGameNode->GetCumRegret(iAction);
		if (actionCumRegret > 0) {
			regretSum += actionCumRegret;
		}
	}
	for (int iAction = 0; iAction < numChildren; iAction++) {
		float cumRegret = treeGameNode->GetCumRegret(iAction);
		float updatedProb = regretSum > 0 ? (std::max(cumRegret, 0.0f) / regretSum) : (1.0f / (float) numChildren);
		treeGameNode->SetCurrStratProb(updatedProb, iAction);
	}
	
	long pChildPos = treeGameNode->mpChildStartOffset;
	for (int iChild = 0; iChild < numActions; iChild++) {
		pChildPos = UpdateStratProbsRecursive(pChildPos);
	}
	return treeGameNode->mpNextNodePos;
}

template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::UpdateStratChanceNode(TreeChanceNode *treeChanceNode) {

	long pChildPos = treeChanceNode->mpChildStartOffset;
	for (int iChild = 0; iChild < treeChanceNode->mNumChildren; iChild++) {
		pChildPos = UpdateStratProbsRecursive(pChildPos);
	}
	return treeChanceNode->mpNextNodePos;
}



template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::CFR(int iterations, float accuracy) {
	
	for (int iCFR = 0; iCFR < iterations; iCFR++) {
		CFRHelper(0, 1.0, 1.0);
		UpdateStratProbsRecursive(0);
	}
	UpdateNashStrategy(0, iterations);
	return;
}

template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::UpdateNashStrategy(long pNodePos, int numIterations) {
	unsigned char nodeIdentifier = (unsigned char) mGameTree[pNodePos];

	if (nodeIdentifier == 'g') {

		TreeGameNode treeGameNode(mGameTree, pNodePos);
		int numChildren = treeGameNode.mNumChildrenOrActions;
		int numActions = numChildren;
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






template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::PrintGameTree() {
	PrintTreeHelper(0);

}

template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline long CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::PrintTreeHelper(long pNodePos) {
	
	unsigned char nodeIdentifier = mGameTree[pNodePos];
	if (nodeIdentifier == 't') {
		return pNodePos + sizeof(byte) + sizeof(float);
	}
	else if (nodeIdentifier == 'g') {
		TreeGameNode treeGameNode(mGameTree, pNodePos);
		std::cout << treeGameNode;
		
		long pChildPos = treeGameNode.mpChildStartOffset;
		for (int iChild = 0; iChild < treeGameNode.mNumChildrenOrActions; iChild++) {
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












