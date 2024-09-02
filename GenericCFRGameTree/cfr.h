#pragma once
#include "pch.h"
#include "framework.h"
#include <cstddef>
#include <concepts>
#include <functional>
#include <utility>
#include <unordered_map>
#include <string>
#include "nodes.h"
#include "cfr_tree_nodes.h"

using byte = TreeUtils::byte;

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode> &&
		 CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode> &&
		 CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode> &&
		 CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode> &&
		 CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass> &&
		 CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
class CfrTree {

	byte* mpGameTree;
	byte* mpRegretTable;
	GameClass* mpStaticGameInfo;
	ChanceNode mStartingChanceNode;

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
		mSearchTreeSize{ 0 }, mInfoSetTableSize{ 0 } {}

	/**
		* @brief Construct the game tree starting from the root chance node.
		*/
	void ConstructTree();

	long TreeSize() { return mSearchTreeSize + mInfoSetTableSize; }

	long SearchTreeSize() { return mSearchTreeSize; }

	long InfoSetTableSize() { return mInfoSetTableSize; }

	void PrintTree();

	void BaseCFR(int iterations);

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

	void PrintTreeRecursive(SearchTreeNode& node);

	float BaseCfrRecursive(
		SearchTreeNode& node, bool isPlayerOne, int iteration,
		float playerOneReachProb, float playerTwoReachProb
	);

	void NewStrategy(InfoSetData& infoSet);
};
			 
/*
#################################
## Public Function Definitions ##
#################################
*/

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ConstructTree() {
	CfrTreeNode *root = new CfrTreeNode(mStartingChanceNode);
	NodeList startingList = NodeList(1, root);
	ConstructTreeHelper(0, 0, startingList);
}


template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
PrintTree() {
	SearchTreeNode rootChance = SearchTreeNode(mpGameTree);
	PrintTreeRecursive(rootChance);
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
BaseCFR(int iterations) {

	SearchTreeNode rootChance = SearchTreeNode(mpGameTree);
	for (int iCfr = 0; iCfr < iterations; iCfr++) {
		BaseCfrRecursive(rootChance, true, iCfr, 1, 1);
		BaseCfrRecursive(rootChance, false, iCfr, 1, 1);
	}
}

/*
##################################
## Private Function Definitions ##
##################################
*/

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
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
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
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
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
inline int CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
ExploreNode(
	CfrTreeNode* searchNode, NodeList& nextChildNodes, InfoSetSizes& infoSetMap
) {
		
	if (searchNode->IsPlayerNode()) {
		
		PlayerNode currNode = searchNode->GetPlayerNode();
		std::vector<Action> actions = currNode.ActionList();
		for (Action a : actions) {
			CfrTreeNode child = currNode.Child(a);

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
		std::vector<CfrClientNode> children = currNode.Children();
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
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
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
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
inline byte* CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetNode(CfrTreeNode* searchNode, byte* nodePos, InfoSetPositions& infoSetPosMap) {
		
	if (searchNode->IsPlayerNode()) {
		PlayerNode currNode = searchNode->GetPlayerNode();
		std::vector<Action> actions = currNode.ActionList();
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
		std::vector<CfrClientNode> children = currNode.Children();
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
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
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
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
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


template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
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
			float childReachProb = childProbabilities.at(childIndex);
			val += childReachProb * childUtil;
			childIndex++;
		}
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
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
NewStrategy(InfoSetData& infoSet) {
	float regretSum = 0;
	for (int iAction = 0; iAction < infoSet.numActions(); iAction++) {
		float actionRegret = infoSet.GetCumulativeRegret(iAction);
		if (actionRegret > 0) {
			regretSum += actionRegret;
		}
	}
	for (int iAction = 0; iAction < infoSet.numActions(); iAction++) {
		float cumRegret = infoSet.GetCumulativeRegret(iAction);
		float updatedProb = regretSum > 0 ? (std::max(cumRegret, 0.0f) / regretSum ) : 1.0 / (float) infoSet.numActions();
		infoSet.SetCurrentStrategy(updatedProb, iAction);
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>&&
CfrConcepts::PlayerNodeActionListFunc<Action, PlayerNode>&&
CfrConcepts::PlayerNodeChildFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::ChanceNodeChildrenFunc<Action, PlayerNode, ChanceNode>&&
CfrConcepts::NeedsUtilityFunc< Action, PlayerNode, ChanceNode, GameClass>&&
CfrConcepts::Hashable< Action, PlayerNode, ChanceNode>
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

