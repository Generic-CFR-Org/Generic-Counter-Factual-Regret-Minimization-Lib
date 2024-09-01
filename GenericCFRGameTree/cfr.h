#pragma once
#include "pch.h"
#include "framework.h"
#include <cstddef>
#include <concepts>
#include <functional>
#include <utility>
#include <unordered_map>
#include "cfr_concepts.h"
#include "nodes.h"
#include "cfr_tree_nodes.h"

namespace Cfr {

	#include <string>

	template<typename Action, typename PlayerNode, typename ChanceNode>
	concept PlayerNodeFuncs = requires( Action a, PlayerNode p, ChanceNode c ) {


		/*PlayerNode must have function IsPlayerOne() that returns a bool for the currently
			acting player*/
		{ p.IsPlayerOne() } -> std::convertible_to<std::vector<bool>>;

		/*Player node must have function that returns a list of actions it can take*/
		{ p.ActionList() } -> std::convertible_to<std::vector<Action>>;

		/*Player Node must have function that returns child node of an action.*/
		{ p.Child(std::declval<Action>) } -> std::convertible_to<TreeNodes::TreeNode<Action, PlayerNode, ChanceNode>>;

		/*Chance Node must have function that returns vector of child nodes*/
		{ c.Children() } -> std::convertible_to<std::vector<TreeNodes::TreeNode<Action, PlayerNode, ChanceNode>>>;

	};

	template<typename Action, typename PlayerNode, typename ChanceNode>
	concept ChanceNodeFuncs = requires( Action a, PlayerNode p, ChanceNode c ) {

		/*Chance Node must have function that returns vector of child nodes*/
		{ c.Children() } -> std::convertible_to<std::vector<TreeNodes::TreeNode<Action, PlayerNode, ChanceNode>>>;

	};


	int playerNodeSize = Cfr::CfrTrees::Search::PlayerNodeSizeInTree();
	int terminalNodeSize = Cfr::CfrTrees::Search::TerminalNodeSizeInTree();

	template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires PlayerNodeFuncs<Action, PlayerNode, ChanceNode> &&
			 ChanceNodeFuncs<Action, PlayerNode, ChanceNode>
	class CfrTree {

		typedef unsigned char byte;

		byte* mpGameTree;
		byte* mpRegretTable;
		GameClass* mpStaticGameInfo;
		ChanceNode mStartingChanceNode;


		typedef TreeNodes::TreeNode<Action, PlayerNode, ChanceNode> CfrTreeNode;

		typedef TreeNodes::PlayerNodeChild<Action, PlayerNode, ChanceNode> PlayerChild;
		typedef TreeNodes::ChanceNodeChild<Action, PlayerNode, ChanceNode> ChanceChild;

		typedef TreeNodes::HistoryNode<Action, PlayerNode, ChanceNode> CfrHistoryNode;
		

		typedef std::pair<PlayerChild, CfrHistoryNode*> PlayerNodePair;
		typedef std::pair<ChanceChild, CfrHistoryNode*> ChanceNodePair;

		typedef std::vector<PlayerNodePair> PlayerChildList;
		typedef std::vector<ChanceNodePair> ChanceChildList;

		typedef std::unordered_map<std::string, int> InfoSetSizes;

		long ConstructTree() {
			return 1;
		}

		long InfoSetSizeAtDepth(InfoSetSizes& infoSets) {
			long totalSize = 0;
			for (auto it : infoSets) {
				totalSize += it.second;
			}
		}

		void ExplorePlayerNodeChildren
		(
			long& cumulativeSize,
			PlayerChildList& playerChildNodes,
			PlayerChildList& nextPlayerChildNodes,
			ChanceChildList& nextChanceChildNodes,
			std::vector<uint8_t>& nodeSizes,
			InfoSetSizes& infoSetSizes
		) 
		{
			for (PlayerNodePair playerNodePair : playerChildNodes) {

				PlayerChild exploredChildNode = playerNodePair.first;
				CfrHistoryNode* exploredHistoryNode = playerNodePair.second;
				if (exploredChildNode.IsPlayerNode()) {

					//Get Actions for current player node to find children.
					PlayerNode currNode = exploredChildNode.ChildPlayerNode();
					std::vector<Action> actions = currNode.ActionList();

					for (Action a : actions) {
						CfrTreeNode child = currNode.Child(a);
						//Once we get the child, convert it into a child node
						PlayerChild playerChild = PlayerChild(child, a);
						//Add to next children.
						nextPlayerChildNodes.push_back(playerChild);
					}
					//Add current node size to nextParentChildren for previous level to use.
					nodeSizes.push_back(playerNodeSize);

					//Find history hash to track # of info sets and their sizes.
					std::string historyHash = exploredHistoryNode->HistoryHash();
					infoSetSizes.insert(historyHash, actions.size());
				}
				else if (exploredChildNode.IsChanceNode()) {
					
					//Add current node size to nextParentChildren for previous level to use.
					ChanceNode currNode = exploredChildNode.ChildChanceNode();
					std::vector<CfrTreeNode> children = currNode.Children();
					int chanceNodeSize = Cfr::CfrTrees::Search::ChanceNodeSizeInTree(children.size());
					nodeSizes.push_back(chanceNodeSize);

					//Add each chance node child to next chance child nodes.
					for (CfrTreeNode child : children) {
						
						ChanceChild chanceChild = ChanceChild(child);
						nextChanceChildNodes.push_back(chanceChild);
					}
				}
				else {

					//If node is terminal. Just add node size to nodeSizes array.
					nodeSizes.push_back(terminalNodeSize);
				}

			}
		}

		void ExploreChanceNodeChildren
		(
			long& cumulativeSize,
			ChanceChildList& chanceChildNodes,
			PlayerChildList& nextPlayerChildNodes,
			ChanceChildList& nextChanceChildNodes,
			std::vector<uint8_t>& nodeSizes,
			InfoSetSizes& infoSetSizes
		) 
		{
			for (ChanceNodePair playerNodePair : chanceChildNodes) {

				ChanceChild exploredChildNode = playerNodePair.first;
				CfrHistoryNode* exploredHistoryNode = playerNodePair.second;
				if (exploredChildNode.IsPlayerNode()) {

					//Get Actions for current player node to find children.
					PlayerNode currNode = exploredChildNode.ChildPlayerNode();
					std::vector<Action> actions = currNode.ActionList();

					for (Action a : actions) {
						CfrTreeNode child = currNode.Child(a);
						//Once we get the child, convert it into a child node
						PlayerChild playerChild = PlayerChild(child, a);
						//Add to next children.
						nextPlayerChildNodes.push_back(playerChild);
					}
					//Add current node size to nextParentChildren for previous level to use.
					nodeSizes.push_back(playerNodeSize);

					//Find history hash to track # of info sets and their sizes.
					std::string historyHash = exploredHistoryNode->HistoryHash();
					infoSetSizes.insert(historyHash, actions.size());
				}
				else if (exploredChildNode.IsChanceNode()) {

					//Add current node size to nextParentChildren for previous level to use.
					ChanceNode currNode = exploredChildNode.ChildChanceNode();
					std::vector<CfrTreeNode> children = currNode.Children();
					int chanceNodeSize = Cfr::CfrTrees::Search::ChanceNodeSizeInTree(children.size());
					nodeSizes.push_back(chanceNodeSize);

					//Add each chance node child to next chance child nodes.
					for (CfrTreeNode child : children) {

						ChanceChild chanceChild = ChanceChild(child);
						nextChanceChildNodes.push_back(chanceChild);
					}
				}
				else {
					//If node is terminal. Just add node size to nodeSizes array.
					nodeSizes.push_back(terminalNodeSize);
				}
			}
		}

		void ConstructInfoSetTable
		(
			long depthOffset, PlayerChildList& playerChildNodes,
			ChanceChildList& chanceChildNodes, 
		)
		{}

		void ConstructTreeHelperBFS
		(
			int depth, long startSearchOffset, long startInfosetOffset,
			PlayerChildList& playerChildNodes,
			ChanceChildList& chanceChildNodes,
			std::vector<uint8_t>& numChildrenPerNode,
		) 
		{
			/*Store next children in arrays*/
			PlayerChildList nextPlayerNodeChildren;
			ChanceChildList nextChanceNodeChildren;

			//Update next offset for next level of the tree.
			long nextSearchOffset = startSearchOffset;

			//Initialize unordered map for current depth to track info set sizes.
			InfoSetSizes infoSetSizes;
			
			ExplorePlayerNodeChildren
			(
				nextSearchOffset, playerChildNodes, nextPlayerNodeChildren,
				nextChanceNodeChildren, numChildrenPerNode, infoSetSizes
			);
			ExploreChanceNodeChildren
			(
				nextSearchOffset, chanceChildNodes, nextPlayerNodeChildren,
				nextChanceNodeChildren, numChildrenPerNode, infoSetSizes
			);

			/*Once all nodes are explored, explore next depth unless there
			  are no more children, in which case we move to tree construction.*/
			if (nextPlayerNodeChildren.size() + nextChanceNodeChildren.size() == 0) {
				this->mpGameTree = new byte[nextOffset];
				this->mpRegretTable = new byte[startInfosetOffset];
			}

			//Set search tree nodes and info sets.


		}
	};







}