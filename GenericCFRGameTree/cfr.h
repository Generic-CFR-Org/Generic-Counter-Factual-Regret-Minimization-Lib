#pragma once
#include <complex>
#include "pch.h"
#include "framework.h"
#include <cstddef>
#include <concepts>
#include <functional>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
#include <random>
#include "nodes.h"
#include "cfr_tree_nodes.h"


using byte = unsigned char;

/**
 * @brief Requirements for client to use the generic cfr tree.
 */
template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
concept GenericCfrRequirements = requires( Action a, PlayerNode p, ChanceNode c, GameClass g ) {
	CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
	CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode, GameClass>&&
	CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode, GameClass>&&
	CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode, GameClass>&&
	CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
	CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>;
};

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
class CfrTree {

	/**
	 * @brief Stores pointer to the search tree constructed by the class.
	 */
	byte* mpGameTree;

	/**
	 * @brief Stores pointer to the regret table constructed by the class.
	 */
	byte* mpRegretTable;

	/**
	 * @brief Stores pointer to the static game metadata provided by the client.
	 */
	GameClass* mpStaticGameInfo;

	/**
	 * @brief Stores Chance Node object representing the root node of the game.
	 */
	ChanceNode mStartingChanceNode;

	/**
	 * @brief Stores the sizes of the search tree and regret table respectively.
	 */
	long mSearchTreeSize;
	long mInfoSetTableSize;

public:
	typedef TreeNode<Action, PlayerNode, ChanceNode> CfrTreeNode;
	typedef ClientNode<Action, PlayerNode, ChanceNode> CfrClientNode;

	typedef std::vector<CfrTreeNode*> NodeList;
	typedef std::vector<CfrTreeNode> HistoryList;
		
	typedef std::unordered_map<std::string, int> InfoSetSizes;
	typedef std::unordered_map<std::string, byte*> InfoSetPositions;

	CfrTree(GameClass* gameInfo, ChanceNode rootNode) :
		mpGameTree{ nullptr }, mpRegretTable{ nullptr },
		mpStaticGameInfo{ gameInfo }, mStartingChanceNode{ rootNode },
		mSearchTreeSize{ 0 }, mInfoSetTableSize{ 0 } {
		std::srand((unsigned) std::time(NULL));
	}

	/**
		* @brief Construct the game tree starting from the root chance node.
		*/
	void ConstructTree();

	/**
	 * @return The combined size of the regret table and search tree in bytes.
	 */
	long TreeSize() { return mSearchTreeSize + mInfoSetTableSize; }

	/**
	 * @return The size of the search tree in bytes
	 */
	long SearchTreeSize() { return mSearchTreeSize; }

	/**
	 * @return The size of the Info Set (Regret) table in bytes.
	 */
	long InfoSetTableSize() { return mInfoSetTableSize; }

	/**
	 * @brief Prints out all nodes in the search tree and info sets in the regret table.
	 */
	void PrintTree();

	/**
	 * @brief Runs CFR on the search tree / regret table, exploring every node
	 *		  in the search tree for each iteration.
	 * @param iterations Number of iterations to update the entire tree.
	 */
	void BaseCFR(int iterations);

	/**
	 * @brief Runs CFR on the search tree / regret table, exploring a single subtree
	 *		  of each chance node in the search tree for each iteration.
	 * @param iterations Number of iterations to update the tree.
	 */
	void ChanceSamplingCFR(int iterations);

	/**
	 * @brief Runs CFR on the search tree / regret table, exploring every node
	 *		  in the search tree for each iteration until desired accuracy is reached.
	 *		  Accuracy is determined by summing the 2 players' value at the root node.
	 * @param iterations Desired accuracy to reach..
	 */
	void BaseCFRwithAccuracy(float accuracy);

	/**
	 * @brief Runs CFR on the search tree / regret table, exploring a single subtree
	 *		  of each chance node in the search tree for each iteration 
			  until desired accuracy is reached.
			  Accuracy is determined by summing the 2 players' value at the root node.
	 * @param iterations Number of iterations to update the tree.
	 */
	void ChanceSamplingCFRwithAccuracy(float accuracy);

private:


	/**
		* @brief Conducts 3 stage routine to preprocess and construct the game tree.
		* Stage 1) a) Find children of all nodes at the current depth,
		*			b) Determine the size of all nodes at the current depth.
		*		    c) Determine the starting search tree & info set offset for next depth.
		* Stage 2) a) Recursively call on Construct Tree Helper at next depth, or (b)
		*			b) Allocate regret table and search tree memory.
		* Stage 3) a) Set all info set nodes at the current depth, record their positions.
		*			b) Set all search tree nodes, using info set positions for player nodes.
		*			c) Deallocate set nodes in the function scope.
		*/
	void ConstructTreeHelper(
		long cumSearchTreeSize, long cumInfoSetTableSize,
		NodeList& nodesToExplore
	);

	/**
		* @brief Function that preprocess tree to find total size ot allcoate.
		*/
	void PreProcessTree(
		NodeList& nodesToExplore, NodeList& nextChildren,
		InfoSetSizes& infoSetSizeMap, long& searchSize, long& infoSetSize
	);

	/**
		* @brief Helper method for PreProcessTree to process a single search node.
		*		  Updates infoSetMap to track number of infosets and their sizes.
		* @return Size of the search node in the search tree.
		*/
	int ExploreNode (
		CfrTreeNode* searchNode, NodeList& nextChildNodes,
		InfoSetSizes& infoSetMap
	);

	/**
		* @brief Sets all nodes in search tree and info set table after memory allocation. 
		*/
	void SetAllNodes(
		long searchOffset, long infoSetOffset,
		NodeList& nodesToSet, InfoSetSizes& infoSetSizeMap
	);

	/**
		* @brief Sets a single node in the search tree.
		* @return Returns pointer to position to set next node.
		*/
	byte* SetNode (
		CfrTreeNode* searchNode, byte* nodePos, 
		InfoSetPositions& infoSetPosMap
	);

	/**
		* @brief Set all info sets in info set table
		*		  Update info set pos map for player nodes in search tree.
		*/
	void SetInfoSets(
		long infoSetOffset, InfoSetSizes& infoSetSizeMap, 
		InfoSetPositions& infoSetPosMap
	);

	/**
		* @brief Set all search nodes in the search tree.
		*/
	void SetSearchNodes(
		long searchOffset, NodeList& nodesToSet,
		InfoSetPositions& infoSetPosMap
	);

	/**
	 * @brief Iterates through all nodes in the search tree and prints
	 *		  relevant information (including info sets)
	 * @param node 
	 */
	void PrintTreeRecursive(SearchTreeNode& node);

	/**
	 * @brief Recursively runs CFR on all nodes in search tree without chance sampling
	 * @return The value of the subgame.
	 */
	float BaseCfrRecursive(
		SearchTreeNode& node, bool isPlayerOne, int iteration,
		float playerOneReachProb, float playerTwoReachProb
	);

	/**
	 * @brief Recursively runs CFR on all nodes in search tree with chance sampling
	 * @return The value of the subgame.
	 */
	float ChanceSamplingCfrRecursive(
		SearchTreeNode& node, bool isPlayerOne, int iteration,
		float playerOneReachProb, float playerTwoReachProb
	);

	/**
	 * @brief Returns a random index of a chance node's child based on
	 *	      the probability of reaching the child.
	 * @param chanceNode 
	 * @return Index to access child node.
	 */
	int SampleChanceNodeIndex(SearchTreeNode& chanceNode);

	/**
	* @brief updates current strategy for an info set during an iteration of CFR.
	*/
	void NewStrategy(InfoSetData& infoSet);
	
	/**
	* @brief updates overall strategy for an info set after all iterations of CFR.
	*/
	void AverageStrategy(
		SearchTreeNode& node, std::unordered_set<byte*> alreadyEvaluated
	);
};
			 
/*
#################################
## Public Function Definitions ##
#################################
*/
template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ConstructTree() {
	CfrTreeNode *root = new CfrTreeNode(mStartingChanceNode);
	NodeList startingList = NodeList(1, root);
	ConstructTreeHelper(0, 0, startingList);
}


template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
PrintTree() {
	SearchTreeNode rootChance = SearchTreeNode(mpGameTree);
	PrintTreeRecursive(rootChance);
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
BaseCFR(int iterations) {

	SearchTreeNode rootChance = SearchTreeNode(mpGameTree);
	for (int iCfr = 0; iCfr < iterations; iCfr++) {
		
		BaseCfrRecursive(rootChance, true, iCfr, 1, 1);
		BaseCfrRecursive(rootChance, false, iCfr, 1, 1);
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ChanceSamplingCFR(int iterations) {
	SearchTreeNode rootChance = SearchTreeNode(mpGameTree);
	for (int iCfr = 0; iCfr < iterations; iCfr++) {

		ChanceSamplingCfrRecursive(rootChance, true, iCfr, 1, 1);
		ChanceSamplingCfrRecursive(rootChance, false, iCfr, 1, 1);
	}
	std::unordered_set<byte*> seenInfoSets;
	AverageStrategy(rootChance, seenInfoSets);
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
BaseCFRwithAccuracy(float accuracy) {

	SearchTreeNode rootChance = SearchTreeNode(mpGameTree);
	int itersPetExploitabilityCheck = 10;
	float exploitability = 100.0;
	int i = 0;
	while (exploitability > accuracy) {
		for (int iCfr = 0; iCfr < itersPetExploitabilityCheck - 1; iCfr++) {

			BaseCfrRecursive(rootChance, true, i, 1, 1);
			BaseCfrRecursive(rootChance, false, i, 1, 1);
			i++;
		}
		//One last iteration to calculate exploitability
		float playerOneEv = BaseCfrRecursive(rootChance, true, i, 1, 1);
		float playerTwoEv = BaseCfrRecursive(rootChance, false, i, 1, 1);
		exploitability = playerOneEv + playerTwoEv;
		i++;
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ChanceSamplingCFRwithAccuracy(float accuracy) {

	SearchTreeNode rootChance = SearchTreeNode(mpGameTree);
	int itersPetExploitabilityCheck = 10;
	float exploitability = 100.0;
	int i = 0;
	while (exploitability > accuracy) {
		for (int iCfr = 0; iCfr < itersPetExploitabilityCheck - 1; iCfr++) {

			ChanceSamplingCfrRecursive(rootChance, true, i, 1, 1);
			ChanceSamplingCfrRecursive(rootChance, false, i, 1, 1);
			i++;
		}
		//One last iteration to calculate exploitability
		float playerOneEv = BaseCfrRecursive(rootChance, true, i, 1, 1);
		float playerTwoEv = BaseCfrRecursive(rootChance, false, i, 1, 1);

		exploitability = playerOneEv + playerTwoEv;

		i++;
	}
	std::unordered_set<byte*> seenInfoSets;
	AverageStrategy(rootChance, seenInfoSets);
}

/*
##################################
## Private Function Definitions ##
##################################
*/


/*
#########################################
## Tree Preprocessers and constructors ##
#########################################
*/

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ConstructTreeHelper(
	long cumSearchTreeSize, long cumInfoSetTableSize,
	NodeList& nodesToExplore
) {

	/*
	########################################################
	# Stage 1: Preprocessing tree to allocate correct size #
	########################################################
	*/

	/*Store next children in arrays*/
	NodeList nextChildren;

	//Initialize unordered map for current depth to track info set sizes.
	InfoSetSizes infoSetSizes;
	//Initialize long values to store size of search / info set trees.
	long currSearchSize = 0;
	long currInfoSetSize = 0;

	//Explore all children and update cumulative size.
	PreProcessTree(nodesToExplore, nextChildren, infoSetSizes,
				   currSearchSize, currInfoSetSize);

	long nextSearchTreeSize = cumSearchTreeSize + currSearchSize;
	long nextInfoSetSize = cumInfoSetTableSize + currInfoSetSize;

	/*
	##########################################
	## Stage 2: Determine if at final depth ##
	##########################################

	If at final depth, allocate both the search tree and Info set table.
	If not, recurse to next depth before node allocation.
	*/

	if (nodesToExplore.size() == 0) {
		this->mpGameTree = new byte[nextSearchTreeSize];
		this->mpRegretTable = new byte[nextInfoSetSize];
		mSearchTreeSize = nextSearchTreeSize;
		mInfoSetTableSize = nextInfoSetSize;
	}
	else {

		ConstructTreeHelper(nextSearchTreeSize,
							nextInfoSetSize, nextChildren);
	}

	/*
	##################################################
	## Stage 3: Set Info Sets and Search Tree Nodes ##
	##################################################

	After recursive call returns, set all nodes as the tree has been
	allocated.
	- Player Nodes are given pointer to their respective info set.
	- Setting nodes updates their parents to their offsets.
	*/
	SetAllNodes(cumSearchTreeSize, cumInfoSetTableSize, nodesToExplore, infoSetSizes);
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
PreProcessTree(
	NodeList& nodesToExplore, NodeList& nextChildren,
	InfoSetSizes& infoSetSizeMap, long& searchSize, long& infoSetSize
) {
	for (CfrTreeNode* currNode : nodesToExplore) {
		searchSize += ExploreNode(currNode, nextChildren, infoSetSizeMap);
	}
	//Update infoset size.
	for (const auto& [key, val] : infoSetSizeMap) {
		infoSetSize += TreeUtils::InfoSetSize(val);
	}
	return;
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline int CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ExploreNode(
	CfrTreeNode* searchNode, NodeList& nextChildNodes, InfoSetSizes& infoSetMap
) {
		
	if (searchNode->IsPlayerNode()) {
		
		PlayerNode currNode = searchNode->GetPlayerNode();
		std::vector<Action> actions = currNode.ActionList(mpStaticGameInfo);
		for (Action a : actions) {
			CfrTreeNode child = currNode.Child(a, mpStaticGameInfo);

			//Allocate child, point to parent, and add to next nodes to explore.
			CfrTreeNode* nextChild = new CfrTreeNode(child, searchNode);
			nextChildNodes.push_back(nextChild);
		}
		//Use node to find history hash set number of actions of the info set.
		std::string historyHash = searchNode->HistoryHash();
		infoSetMap.insert({ historyHash, actions.size() });

		//Return size of the node.
		return TreeUtils::PLAYER_NODE_SIZE;
	}
	else if (searchNode->IsChanceNode()) {
		
		ChanceNode currNode = searchNode->GetChanceNode();
		std::vector<CfrClientNode> children = currNode.Children(mpStaticGameInfo);
		//Add each chance node child to next chance child nodes.
		for (CfrClientNode child : children) {

			CfrTreeNode *nextChild = new CfrTreeNode(child, searchNode);
			nextChildNodes.push_back(nextChild);
		}
		//Return size of the chance node in the search tree.
		return TreeUtils::ChanceNodeSizeInTree(children.size());
	}
	//Else if terminal, return static terminal node size.
	
	return TreeUtils::TERMINAL_SIZE;
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetAllNodes(
	long searchOffset, long infoSetOffset,
	NodeList& nodesToSet, InfoSetSizes& infoSetSizeMap
	) {
	//Initialize map to store position of each info set once set.
	InfoSetPositions infoSetPosMap;

	//Set all Info Set nodes.
	SetInfoSets(infoSetOffset, infoSetSizeMap, infoSetPosMap);

	//Use info set position map to set all search tree nodes.
	SetSearchNodes(searchOffset, nodesToSet, infoSetPosMap);
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline byte* CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetNode(CfrTreeNode* searchNode, byte* nodePos, InfoSetPositions& infoSetPosMap) {
		
	if (searchNode->IsPlayerNode()) {
		PlayerNode currNode = searchNode->GetPlayerNode();
		std::vector<Action> actions = currNode.ActionList(mpStaticGameInfo);
		int numChildren = actions.size();
		byte* childStartOffset = searchNode->GetChildOffset();
		bool isPlayerOne = currNode.IsPlayerOne();

		//Use history hash to find info set position.
		std::string historyHash = searchNode->HistoryHash();
		byte* infoSetPos = infoSetPosMap.at(historyHash);

		//Update parent child start offset.
		searchNode->UpdateParentOffset(nodePos);

		//Once node is set, it can be safely deleted.
		delete searchNode;

		//Set player node and return byte* to next node.
		return TreeUtils::SetPlayerNode(nodePos, numChildren, childStartOffset, isPlayerOne, infoSetPos);
	}
	else if (searchNode->IsChanceNode()) {
		ChanceNode currNode = searchNode->GetChanceNode();
		std::vector<CfrClientNode> children = currNode.Children(mpStaticGameInfo);
		std::vector<float> probList = ToFloatList(children);
		byte* childStartOffset = searchNode->GetChildOffset();

		//Update parent child start offset.
		searchNode->UpdateParentOffset(nodePos);

		//Once node is set, it can be safely deleted.
		delete searchNode;

		//Return size of the chance node in the search tree.
		return TreeUtils::SetChanceNode(nodePos, childStartOffset, probList);
	}
	//Else if terminal, return static terminal node size.
	HistoryList historyList = searchNode->HistoryList();
	float utility = mpStaticGameInfo->UtilityFunc(historyList);

	//Update parent child start offset.
	searchNode->UpdateParentOffset(nodePos);

	//Once node is set, it can be safely deleted.
	delete searchNode;

	return TreeUtils::SetTerminalNode(nodePos, utility);
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetInfoSets(
	long infoSetOffset,
	InfoSetSizes& infoSetSizeMap,
	InfoSetPositions& infoSetPosMap
) {
	byte* currOffset = mpRegretTable + infoSetOffset;
	for (const auto& [key, val] : infoSetSizeMap) {
		infoSetPosMap.insert({ key, currOffset });
		currOffset = TreeUtils::SetInfoSetNode(currOffset, val);
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetSearchNodes(
	long searchOffset, NodeList& nodesToSet,
	InfoSetPositions& infoSetPosMap
) {
	byte* currPos = mpGameTree + searchOffset;
	for (CfrTreeNode* nodeToSet : nodesToSet) {
		currPos = SetNode(nodeToSet, currPos, infoSetPosMap);
	}
}


/*
############################################
## Counter Factual Regret Solver Methods  ##
############################################
*/

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline float CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
BaseCfrRecursive(
	SearchTreeNode& node, bool isPlayerOne, int iteration,
	float playerOneReachProb, float playerTwoReachProb
) {
	
	if (node.IsTerminalNode()) {
		return node.Utility();
	}
	else if (node.IsChanceNode()) {
		float val = 0;
		int childIndex = 0;
		std::vector<float> childProbabilities = node.ChildProbabilities();
		std::vector<SearchTreeNode> children = node.AllChildren();
		for (SearchTreeNode& child : children) {
			float childUtil = BaseCfrRecursive(child, isPlayerOne, iteration, playerOneReachProb, playerTwoReachProb);
			float childReachProb = childProbabilities[childIndex];
			val += childReachProb * childUtil;
			childIndex++;
		}
		return val;
	}
	else {
		float val = 0;
		int numChildren = node.NumChildren();
		std::vector<SearchTreeNode> children = node.AllChildren();
		std::vector<float> childUtilities(numChildren, 0);
		InfoSetData infoSet = InfoSetData(node.InfoSetPosition());
		for (int iAction = 0; iAction < numChildren; iAction++) {
			float currStratProb = infoSet.GetCurrentStrategy(iAction);
			float childUtility;
			if (node.IsPlayerOne()) {
				
				childUtility = BaseCfrRecursive(children.at(iAction), isPlayerOne,
												iteration, currStratProb * playerOneReachProb,
												playerTwoReachProb);
			}
			else {
				childUtility = BaseCfrRecursive(children.at(iAction), isPlayerOne,
												iteration, playerOneReachProb,
												currStratProb * playerTwoReachProb);
			}
			childUtilities.at(iAction) = childUtility;
			val += currStratProb * childUtility;
		}
		if (node.IsPlayerOne() == isPlayerOne) {
			float regretProb;
			float stratProb;
			if (isPlayerOne) {
				regretProb = playerTwoReachProb;
				stratProb = playerOneReachProb;
			}
			else {
				regretProb = playerOneReachProb;
				stratProb = playerTwoReachProb;
			}
			for (int iAction = 0; iAction < numChildren; iAction++) {
				infoSet.AddToCumulativeRegret(regretProb * (childUtilities.at(iAction) - val), iAction);
				infoSet.AddToCumulativeStrategy(stratProb * infoSet.GetCurrentStrategy(iAction), iAction);
			}
			NewStrategy(infoSet);
		}
		return val;
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline float CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ChanceSamplingCfrRecursive(
	SearchTreeNode& node, bool isPlayerOne, int iteration,
	float playerOneReachProb, float playerTwoReachProb
) {

	if (node.IsTerminalNode()) {
		return node.Utility();
	}
	else if (node.IsChanceNode()) {
		
		
		std::vector<SearchTreeNode> children = node.AllChildren();

		//Sample child state using node's probability list.
		int sampleIndex = SampleChanceNodeIndex(node);
		return ChanceSamplingCfrRecursive(children.at(sampleIndex), isPlayerOne, iteration, playerOneReachProb, playerTwoReachProb);
	}
	else {
		float val = 0;
		int numChildren = node.NumChildren();
		std::vector<SearchTreeNode> children = node.AllChildren();
		std::vector<float> childUtilities(numChildren, 0);
		InfoSetData infoSet = InfoSetData(node.InfoSetPosition());
		for (int iAction = 0; iAction < numChildren; iAction++) {
			float currStratProb = infoSet.GetCurrentStrategy(iAction);
			float childUtility;
			if (node.IsPlayerOne()) {

				childUtility = BaseCfrRecursive(children.at(iAction), isPlayerOne,
												iteration, currStratProb * playerOneReachProb,
												playerTwoReachProb);
			}
			else {
				childUtility = BaseCfrRecursive(children.at(iAction), isPlayerOne,
												iteration, playerOneReachProb,
												currStratProb * playerTwoReachProb);
			}
			childUtilities.at(iAction) = childUtility;
			val += currStratProb * childUtility;
		}
		if (node.IsPlayerOne() == isPlayerOne) {
			float regretProb;
			float stratProb;
			if (isPlayerOne) {
				regretProb = playerTwoReachProb;
				stratProb = playerOneReachProb;
			}
			else {
				regretProb = playerOneReachProb;
				stratProb = playerTwoReachProb;
			}
			for (int iAction = 0; iAction < numChildren; iAction++) {
				infoSet.AddToCumulativeRegret(regretProb * ( childUtilities.at(iAction) - val ), iAction);
				infoSet.AddToCumulativeStrategy(stratProb * infoSet.GetCurrentStrategy(iAction), iAction);
			}
			NewStrategy(infoSet);
		}
		return val;
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline int CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SampleChanceNodeIndex(SearchTreeNode& chanceNode) {
	
	float randFloat = static_cast<float>(std::rand()) / RAND_MAX;
	std::vector<float> cumulativeProbs = chanceNode.CumulativeChildProbs();
	
	auto it = std::upper_bound(cumulativeProbs.begin(), cumulativeProbs.end(), randFloat);
	return it - cumulativeProbs.begin() - 1;
}



template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
NewStrategy(InfoSetData& infoSet) {
	float regretSum = 0;
	for (int iAction = 0; iAction < infoSet.numActions(); iAction++) {
		float actionRegret = infoSet.GetCumulativeRegret(iAction);
		regretSum += actionRegret;
		if (actionRegret > 0) {
			infoSet.SetCurrentStrategy(actionRegret, iAction);
		}
		else {
			infoSet.SetCurrentStrategy(0.0, iAction);
		}
	}
	float uniformProb = 1.0 / (float) infoSet.numActions();
	for (int iAction = 0; iAction < infoSet.numActions(); iAction++) {
		if (regretSum > 0) {
			float currStrat = infoSet.GetCurrentStrategy(iAction);
			infoSet.SetCurrentStrategy(currStrat / regretSum, iAction);
		}
		else {
			infoSet.SetCurrentStrategy(uniformProb, iAction);
		}
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
AverageStrategy(SearchTreeNode& node, std::unordered_set<byte*> alreadyEvaluated) {
	if (node.IsTerminalNode()) {
		return;
	}
	else if (node.IsChanceNode()) {

		std::vector<SearchTreeNode> children = node.AllChildren();
		for (int iChild = 0; iChild < children.size(); iChild++) {
			AverageStrategy(children.at(iChild), alreadyEvaluated);
		}

	}
	else {
		std::vector<SearchTreeNode> children = node.AllChildren();
		byte* infoSetPtr = node.InfoSetPosition();
		if (!alreadyEvaluated.contains(infoSetPtr)) {
			InfoSetData infoSet = node.InfoSetPosition();
			float normalizingSum = 0;
			for (int iAction = 0; iAction < infoSet.numActions(); iAction++) {
				normalizingSum += infoSet.GetCumulativeStrategy(iAction);
			}
			for (int iAction = 0; iAction < infoSet.numActions(); iAction++) {
				if (normalizingSum > 0) {
					float currStratSum = infoSet.GetCumulativeStrategy(iAction);
					infoSet.SetCurrentStrategy(currStratSum / normalizingSum, iAction);
				}
				else {
					infoSet.SetCurrentStrategy(1.0 / infoSet.numActions(), iAction);
				}
			}
			alreadyEvaluated.insert(infoSetPtr);
		}
	
		for (int iChild = 0; iChild < children.size(); iChild++) {
			AverageStrategy(children.at(iChild), alreadyEvaluated);
		}
	}
}


/*
########################################
## Client UI Interface Helper Methods ##
########################################
*/


template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
PrintTreeRecursive(SearchTreeNode& node) {
	
	std::cout << node;
	if (node.IsPlayerNode() || node.IsChanceNode()) {
		
		std::vector<SearchTreeNode> children = node.AllChildren();
		for (SearchTreeNode& childNode : children) {
			PrintTreeRecursive(childNode);
		}
	}
	return;
}

