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
#include <ranges>

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
	long long PreProcessTree(
		InfoSetSizes& infoSetSizeMap, long long& infoSetSize, std::unordered_map<int, long long>& depthMap, int currDepth
	);

	/**
		* @brief Helper method for PreProcessTree to process a single search node.
		*		  Updates infoSetMap to track number of infosets and their sizes.
		* @return Size of the search node in the search tree.
		*/
	long long ExploreNode(
		CfrTreeNode* searchNode,
		InfoSetSizes& infoSetMap, std::unordered_map<int, long long>& depthMapSize, int currDepth
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
	void SetNode(
		CfrTreeNode* searchNode,
		int depth, std::unordered_map<int, long long>& cumulativeOffsets, InfoSetPositions& infoSetPosMap
	);

	/**
		* @brief Set all info sets in info set table
		*		  Update info set pos map for player nodes in search tree.
		*/
	void SetInfoSets(
		InfoSetSizes& infoSetSizeMap,
		InfoSetPositions& infoSetPosMap
	) const;

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
	
	/*
	########################################################
	# Stage 1: Preprocessing tree to allocate correct size #
	########################################################
	*/
	//Initialize Root node.
	CfrTreeNode* root = new CfrTreeNode(mStartingChanceNode);

	//Initialize unordered map for current depth to track info set sizes.
	InfoSetSizes infoSetSizes;

	//Initialize unordered map to track size at each depth of the tree.
	std::unordered_map<int, long long> depthSizes;

	//Explore all children and update info set and depth size maps.
	long long searchTreeSize = ExploreNode(root, infoSetSizes, depthSizes, 0);

	//Get info set size from the infoSetSizes map.
	long long infoSetSize = 0;
	for (const auto& val : infoSetSizes | std::views::values) {
		infoSetSize += TreeUtils::InfoSetSize(val);
	}


	/*
	##############################
	## Stage 2: Allocate memory ##
	##############################
	*/

	this->mpGameTree = new byte[searchTreeSize];
	this->mpRegretTable = new byte[infoSetSize];
	mSearchTreeSize = searchTreeSize;
	mInfoSetTableSize = infoSetSize;
	

	/*
	########################################################
	## Stage 3: Set Info Sets and update their positions  ##
	########################################################
	*/

	InfoSetPositions info_set_positions;
	SetInfoSets(infoSetSizes, info_set_positions);

	/*
	####################################
	## Stage 3: Set Search Tree nodes ##
	####################################
	*/

	//Generate cumulative offset map to find a node's appropriate position in the search tree.
	long long offsetAtDepth = 0;
	std::unordered_map<int, long long> depthOffsets;
	depthOffsets[0] = offsetAtDepth;
	int iDepth = 1;
	for (const auto& val : depthSizes | std::views::values) {
		offsetAtDepth += val;
		depthOffsets[iDepth] = offsetAtDepth;
		iDepth++;
	}

	//Set nodes 
	SetNode(root, 0, depthOffsets, info_set_positions);

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
inline long long CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ExploreNode(
	CfrTreeNode* searchNode, InfoSetSizes& infoSetMap,
	std::unordered_map<int, long long>& depthMapSize, int currDepth
) {

	if (static_cast<int>( depthMapSize.size()) <= currDepth)
	{
		depthMapSize.insert({ currDepth, 0 });
	}
	long currNodeSize;
	long subTreeSize = 0;
	if (searchNode->IsPlayerNode()) {
		
		PlayerNode currNode = searchNode->GetPlayerNode();
		std::vector<Action> actions = currNode.ActionList(mpStaticGameInfo);
		currNodeSize = TreeUtils::PLAYER_NODE_SIZE;
		for (Action a : actions) {
			CfrTreeNode child = currNode.Child(a, mpStaticGameInfo);

			//Allocate child, and explore it.
			CfrTreeNode* nextChild = new CfrTreeNode(child, searchNode);
			subTreeSize += ExploreNode(nextChild, infoSetMap, depthMapSize, currDepth + 1);
			delete nextChild;
		}
		//Use node to find history hash set number of actions of the info set.
		std::string historyHash = searchNode->HistoryHash();
		infoSetMap.insert({ historyHash, actions.size() });

		
	}
	else if (searchNode->IsChanceNode()) {
		
		ChanceNode currNode = searchNode->GetChanceNode();
		std::vector<CfrClientNode> children = currNode.Children(mpStaticGameInfo);
		//Add each chance node child to next chance child nodes.
		currNodeSize = TreeUtils::ChanceNodeSizeInTree(children.size());
		for (const CfrClientNode& child : children) {

			CfrTreeNode *nextChild = new CfrTreeNode(child, searchNode);
			subTreeSize += ExploreNode(nextChild, infoSetMap, depthMapSize, currDepth + 1);
			delete nextChild;
		}
	}
	else
	{
		//Else if terminal, return static terminal node size.
		currNodeSize = TreeUtils::TERMINAL_SIZE;
	}

	long long oldDepthSize = depthMapSize.at(currDepth);
	depthMapSize.insert_or_assign(currDepth, oldDepthSize + currNodeSize);

	return subTreeSize + currNodeSize;
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetNode(
	CfrTreeNode* searchNode, int depth,
	std::unordered_map<int, long long>& cumulativeOffsets,
	InfoSetPositions& infoSetPosMap
) {

	long long childNodeSize;
	byte* currOffset = mpGameTree + cumulativeOffsets[depth];
	if (searchNode->IsPlayerNode()) {

		PlayerNode currNode = searchNode->GetPlayerNode();
		std::vector<Action> actions = currNode.ActionList(mpStaticGameInfo);
		int numChildren = actions.size();

		/*Recursive over all children to get child offset*/
		for (Action a : actions)
		{
			CfrTreeNode child = currNode.Child(a, mpStaticGameInfo);
			CfrTreeNode* nextChild = new CfrTreeNode(child, searchNode);
			SetNode(nextChild, depth + 1, cumulativeOffsets, infoSetPosMap);
		}

		byte* childStartOffset = searchNode->GetChildOffset();
		bool isPlayerOne = currNode.IsPlayerOne();

		//Use history hash to find info set position.
		std::string historyHash = searchNode->HistoryHash();
		byte* infoSetPos = infoSetPosMap.at(historyHash);

		//Update parent child start offset.
		searchNode->UpdateParentOffset(currOffset);

		//Once node is set, it can be safely deleted.
		delete searchNode;

		//Set player node
		TreeUtils::SetPlayerNode(currOffset, numChildren, childStartOffset, isPlayerOne, infoSetPos);
		childNodeSize =  TreeUtils::PLAYER_NODE_SIZE;
	}
	else if (searchNode->IsChanceNode()) {

		ChanceNode currNode = searchNode->GetChanceNode();
		std::vector<CfrClientNode> children = currNode.Children(mpStaticGameInfo);
		std::vector<float> probList = ToFloatList(children);

		for (const CfrClientNode& child : children) {

			CfrTreeNode* nextChild = new CfrTreeNode(child, searchNode);
			SetNode(nextChild, depth + 1, cumulativeOffsets, infoSetPosMap);
		}

		byte* childStartOffset = searchNode->GetChildOffset();

		//Update parent child start offset.
		searchNode->UpdateParentOffset(currOffset);

		//Set chance node in the game tree.
		TreeUtils::SetChanceNode(currOffset, childStartOffset, probList);

		childNodeSize = TreeUtils::ChanceNodeSizeInTree(children.size());
		//Once node is set, it can be safely deleted.
		delete searchNode;
	}
	else
	{
		//Else set terminal node.
		HistoryList historyList = searchNode->HistoryList();
		float utility = mpStaticGameInfo->UtilityFunc(historyList);

		//Update parent child start offset.
		searchNode->UpdateParentOffset(currOffset);

		TreeUtils::SetTerminalNode(currOffset, utility);

		//Once node is set, it can be safely deleted.
		delete searchNode;
		childNodeSize = TreeUtils::TERMINAL_SIZE;
	}
	//Update cumulative offset map.
	cumulativeOffsets[depth] += childNodeSize;
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetInfoSets(
	InfoSetSizes& infoSetSizeMap,
	InfoSetPositions& infoSetPosMap
) const
{
	byte* currOffset = mpRegretTable;
	for (const auto& [key, val] : infoSetSizeMap) {
		infoSetPosMap.insert({ key, currOffset });
		currOffset = TreeUtils::SetInfoSetNode(currOffset, val);
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

