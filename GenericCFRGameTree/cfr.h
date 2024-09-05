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


using Byte = unsigned char;

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
	Byte* game_tree_;

	/**
	 * @brief Stores pointer to the regret table constructed by the class.
	 */
	Byte* regret_table_;

	/**
	 * @brief Stores pointer to the static game metadata provided by the client.
	 */
	GameClass* static_game_info_;

	/**
	 * @brief Stores Chance Node object representing the root node of the game.
	 */
	ChanceNode starting_chance_node_;

	/**
	 * @brief Stores the sizes of the search tree and regret table respectively.
	 */
	long long search_tree_size_;
	long long info_set_table_size_;

public:
	typedef TreeNode<Action, PlayerNode, ChanceNode> CfrTreeNode;
	typedef ClientNode<Action, PlayerNode, ChanceNode> CfrClientNode;

	typedef std::vector<CfrTreeNode*> NodeList;
	typedef std::vector<CfrTreeNode> HistoryList;
		
	typedef std::unordered_map<std::string, int> InfoSetSizes;
	typedef std::unordered_map<std::string, Byte*> InfoSetPositions;

	CfrTree(GameClass* gameInfo, ChanceNode rootNode) :
		game_tree_{ nullptr }, regret_table_{ nullptr },
		static_game_info_{ gameInfo }, starting_chance_node_{ rootNode },
		search_tree_size_{ 0 }, info_set_table_size_{ 0 }
	{
		std::srand(static_cast<unsigned>(std::time(nullptr)));
	}

	/**
		* @brief Construct the game tree starting from the root chance node.
		*/
	void ConstructTree();

	/**
	 * @return The combined size of the regret table and search tree in bytes.
	 */
	long long TreeSize() const { return search_tree_size_ + info_set_table_size_; }

	/**
	 * @return The size of the search tree in bytes
	 */
	long long SearchTreeSize() const { return search_tree_size_; }

	/**
	 * @return The size of the Info Set (Regret) table in bytes.
	 */
	long long InfoSetTableSize() const { return info_set_table_size_; }

	/**
	 * @brief Prints out all nodes in the search tree and info sets in the regret table.
	 */
	void PrintTree() const;

	/**
	 * @brief Runs CFR on the search tree / regret table, exploring every node
	 *		  in the search tree for each iteration.
	 * @param iterations Number of iterations to update the entire tree.
	 */
	void CFR(int iterations);

	/**
	 * @brief Runs CFR on the search tree / regret table, exploring a single subtree
	 *		  of each chance node in the search tree for each iteration.
	 * @param iterations Number of iterations to update the tree.
	 */
	void MCCFR(int iterations);

	/**
	 * @brief Runs CFR on the search tree / regret table, exploring every node
	 *		  in the search tree for each iteration until desired accuracy is reached.
	 *		  Accuracy is determined by summing the 2 players' value at the root node.
	 * @param accuracy  Desired distance from nash equilibrium to reach.
	 */
	void CFR_ToAccuracy(float accuracy);

	/**
	 * @brief Runs CFR on the search tree / regret table, exploring a single subtree
	 *		  of each chance node in the search tree for each iteration 
			  until desired accuracy is reached.
			  Accuracy is determined by summing the 2 players' value at the root node.
	 * @param accuracy Desired distance from nash equilibrium to reach.
	 */
	void MCCFR_ToAccuracy(float accuracy);

private:

	/**
		* @brief Helper method for PreProcessTree to process a single search node.
		*		  Updates infoSetMap to track number of info sets and their sizes.
		* @return Size of the search node in the search tree.
		*/
	long long ExploreNode(
		CfrTreeNode* search_node, InfoSetSizes& info_set_map,
		std::unordered_map<int, long long>& depth_map_size,
		int curr_depth
	);

	/**
		* @brief Sets a single node in the search tree.
		* @return Returns pointer to position to set next node.
		*/
	void SetNode(
		CfrTreeNode* search_node, int depth,
		std::unordered_map<int, long long>& cumulative_offsets,
		InfoSetPositions& info_set_pos_map
	);

	/**
		* @brief Set all info sets in info set table
		*		  Update info set pos map for player nodes in search tree.
		*/
	void SetInfoSets(
		InfoSetSizes& info_set_size_map,
		InfoSetPositions& info_set_pos_map
	) const;


	/**
	 * @brief Iterates through all nodes in the search tree and prints
	 *		  relevant information (including info sets)
	 * @param node 
	 */
	static void PrintTreeRecursive(const SearchTreeNode& node);

	/**
	 * @brief Recursively runs CFR on all nodes in search tree
	 *		  with or without chance sampling.
	 * @return The value of the subtree of a Search Tree Node.
	 */
	static float WalkTree(
		SearchTreeNode& node, bool is_player_one, int iteration,
		float player_one_reach_prob, float player_two_reach_prob, bool with_sampling
	);


	/**
	* @brief updates current strategy for an info set during an iteration of CFR.
	*/
	static void RegretMatching(InfoSetData& info_set);
	
	/**
	* @brief updates overall strategy for an info set after all iterations of CFR.
	*/
	static void AverageStrategy(
		const SearchTreeNode& node, std::unordered_set<Byte*> already_evaluated
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
	CfrTreeNode* root = new CfrTreeNode(starting_chance_node_);

	//Initialize unordered map for current depth to track info set sizes.
	InfoSetSizes info_set_sizes;

	//Initialize unordered map to track size at each depth of the tree.
	std::unordered_map<int, long long> depth_sizes;

	//Explore all children and update info set and depth size maps.
	long long search_tree_size = ExploreNode(root, info_set_sizes, depth_sizes, 0);

	//Get info set size from the infoSetSizes map.
	long long info_set_size = 0;
	for (const auto& val : info_set_sizes | std::views::values) {
		info_set_size += TreeUtils::InfoSetSize(val);
	}
	/*
	##############################
	## Stage 2: Allocate memory ##
	##############################
	*/

	this->game_tree_ = new Byte[search_tree_size];
	this->regret_table_ = new Byte[info_set_size];
	search_tree_size_ = search_tree_size;
	info_set_table_size_ = info_set_size;

	/*
	########################################################
	## Stage 3: Set Info Sets and update their positions  ##
	########################################################
	*/

	InfoSetPositions info_set_positions;
	SetInfoSets(info_set_sizes, info_set_positions);

	/*
	####################################
	## Stage 3: Set Search Tree nodes ##
	####################################
	*/

	//Generate cumulative offset map to find a node's appropriate position in the search tree.
	long long offset_at_depth = 0;
	std::unordered_map<int, long long> depth_offsets;
	depth_offsets[0] = offset_at_depth;
	int i_depth = 1;
	for (const auto& val : depth_sizes | std::views::values) {
		offset_at_depth += val;
		depth_offsets[i_depth] = offset_at_depth;
		i_depth++;
	}
	//Set nodes 
	SetNode(root, 0, depth_offsets, info_set_positions);

}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
PrintTree() const
{
	SearchTreeNode root_chance = SearchTreeNode(game_tree_);
	PrintTreeRecursive(root_chance);
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
CFR(int iterations) {

	SearchTreeNode root_chance = SearchTreeNode(game_tree_);
	for (int i_cfr = 0; i_cfr < iterations; i_cfr++) {
		
		WalkTree(root_chance, true, i_cfr, 1, 1, false);
		WalkTree(root_chance, false, i_cfr, 1, 1, false);
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
MCCFR(int iterations) {
	SearchTreeNode root_chance = SearchTreeNode(game_tree_);
	for (int i_cfr = 0; i_cfr < iterations; i_cfr++) {

		WalkTree(root_chance, true, i_cfr, 1, 1, true);
		WalkTree(root_chance, false, i_cfr, 1, 1, true);
	}
	std::unordered_set<Byte*> seen_info_sets;
	AverageStrategy(root_chance, seen_info_sets);
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
CFR_ToAccuracy(float accuracy) {

	SearchTreeNode root_chance = SearchTreeNode(game_tree_);
	int iters_pet_exploitability_check = 10;
	float exploitability = 100.0;
	int i = 0;
	while (exploitability > accuracy) {
		for (int iCfr = 0; iCfr < iters_pet_exploitability_check - 1; iCfr++) {

			WalkTree(root_chance, true, i, 1, 1, false);
			WalkTree(root_chance, false, i, 1, 1, false);
			i++;
		}
		//One last iteration to calculate exploit-ability
		WalkTree(root_chance, true, i, 1, 1, false);
		WalkTree(root_chance, false, i, 1, 1, false);
		
		i++;
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
MCCFR_ToAccuracy(float accuracy) {

	SearchTreeNode root_chance = SearchTreeNode(game_tree_);
	const int iters_pet_exploitability_check = root_chance.NumChildren() * 5;
	float exploitability = std::numeric_limits<float>::infinity();
	int i = 0;
	while (exploitability > accuracy) {
		for (int i_cfr = 0; i_cfr < iters_pet_exploitability_check - 1; i_cfr++) {

			WalkTree(root_chance, true, i, 1, 1, true);
			WalkTree(root_chance, false, i, 1, 1, true);
			i++;
		}
		//One last iteration to calculate exploit ability
		float player_one_ev = WalkTree(root_chance, true, i, 1, 1, true);
		float playerTwoEv = WalkTree(root_chance, false, i, 1, 1, true);
		
		exploitability = player_one_ev - playerTwoEv;
		i++;
	}
	std::unordered_set<Byte*> seen_info_sets;
	AverageStrategy(root_chance, seen_info_sets);
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
	CfrTreeNode* search_node, InfoSetSizes& info_set_map,
	std::unordered_map<int, long long>& depth_map_size, int curr_depth
) {

	if (static_cast<int>( depth_map_size.size()) <= curr_depth)
	{
		depth_map_size.insert({ curr_depth, 0 });
	}
	long curr_node_size;
	long sub_tree_size = 0;
	if (search_node->IsPlayerNode()) {
		
		PlayerNode curr_node = search_node->GetPlayerNode();
		std::vector<Action> actions = curr_node.ActionList(static_game_info_);
		curr_node_size = TreeUtils::kPlayerNodeSize;
		for (Action a : actions) {
			CfrTreeNode child = curr_node.Child(a, static_game_info_);

			//Allocate child, and explore it.
			CfrTreeNode* next_child = new CfrTreeNode(child, search_node);
			sub_tree_size += ExploreNode(next_child, info_set_map, depth_map_size, curr_depth + 1);
			delete next_child;
		}
		//Use node to find history hash set number of actions of the info set.
		std::string history_hash = search_node->HistoryHash();
		info_set_map.insert({ history_hash, actions.size() });

		
	}
	else if (search_node->IsChanceNode()) {
		
		ChanceNode currNode = search_node->GetChanceNode();
		std::vector<CfrClientNode> children = currNode.Children(static_game_info_);
		//Add each chance node child to next chance child nodes.
		curr_node_size = TreeUtils::ChanceNodeSizeInTree(children.size());
		for (const CfrClientNode& child : children) {

			CfrTreeNode *next_child = new CfrTreeNode(child, search_node);
			sub_tree_size += ExploreNode(next_child, info_set_map, depth_map_size, curr_depth + 1);
			delete next_child;
		}
	}
	else
	{
		//Else if terminal, return static terminal node size.
		curr_node_size = TreeUtils::kTerminalSize;
	}

	long long old_depth_size = depth_map_size.at(curr_depth);
	depth_map_size.insert_or_assign(curr_depth, old_depth_size + curr_node_size);

	return sub_tree_size + curr_node_size;
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetNode(
	CfrTreeNode* search_node, int depth,
	std::unordered_map<int, long long>& cumulative_offsets,
	InfoSetPositions& info_set_pos_map
) {

	long long childNodeSize;
	Byte* curr_offset = game_tree_ + cumulative_offsets[depth];
	if (search_node->IsPlayerNode()) {

		PlayerNode currNode = search_node->GetPlayerNode();
		std::vector<Action> actions = currNode.ActionList(static_game_info_);
		int numChildren = actions.size();

		/*Recursive over all children to get child offset*/
		for (Action a : actions)
		{
			CfrTreeNode child = currNode.Child(a, static_game_info_);
			CfrTreeNode* nextChild = new CfrTreeNode(child, search_node);
			SetNode(nextChild, depth + 1, cumulative_offsets, info_set_pos_map);
		}

		Byte* child_start_offset = search_node->GetChildOffset();
		bool is_player_one = currNode.IsPlayerOne();

		//Use history hash to find info set position.
		std::string history_hash = search_node->HistoryHash();
		Byte* info_set_pos = info_set_pos_map.at(history_hash);

		//Update parent child start offset.
		search_node->UpdateParentOffset(curr_offset);

		//Once node is set, it can be safely deleted.
		delete search_node;

		//Set player node
		TreeUtils::SetPlayerNode(curr_offset, numChildren, child_start_offset, is_player_one, info_set_pos);
		childNodeSize =  TreeUtils::kPlayerNodeSize;
	}
	else if (search_node->IsChanceNode()) {

		ChanceNode curr_node = search_node->GetChanceNode();
		std::vector<CfrClientNode> children = curr_node.Children(static_game_info_);
		std::vector<float> probList = ToFloatList(children);

		for (const CfrClientNode& child : children) {

			CfrTreeNode* next_child = new CfrTreeNode(child, search_node);
			SetNode(next_child, depth + 1, cumulative_offsets, info_set_pos_map);
		}

		Byte* child_start_offset = search_node->GetChildOffset();

		//Update parent child start offset.
		search_node->UpdateParentOffset(curr_offset);

		//Set chance node in the game tree.
		TreeUtils::SetChanceNode(curr_offset, child_start_offset, probList);

		childNodeSize = TreeUtils::ChanceNodeSizeInTree(children.size());
		//Once node is set, it can be safely deleted.
		delete search_node;
	}
	else
	{
		//Else set terminal node.
		HistoryList history_list = search_node->HistoryList();
		float utility = static_game_info_->UtilityFunc(history_list);

		//Update parent child start offset.
		search_node->UpdateParentOffset(curr_offset);

		TreeUtils::SetTerminalNode(curr_offset, utility);

		//Once node is set, it can be safely deleted.
		delete search_node;
		childNodeSize = TreeUtils::kTerminalSize;
	}
	//Update cumulative offset map.
	cumulative_offsets[depth] += childNodeSize;
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
SetInfoSets(
	InfoSetSizes& info_set_size_map,
	InfoSetPositions& info_set_pos_map
) const
{
	Byte* curr_offset = regret_table_;
	for (const auto& [key, val] : info_set_size_map) {
		info_set_pos_map.insert({ key, curr_offset });
		curr_offset = TreeUtils::SetInfoSetNode(curr_offset, val);
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
WalkTree(
	SearchTreeNode& node, bool is_player_one, int iteration,
	float player_one_reach_prob, float player_two_reach_prob, bool with_sampling
) {
	
	if (node.IsTerminalNode()) {
		return node.Utility();
	}
	else if (node.IsChanceNode()) {

		if (with_sampling)
		{
			SearchTreeNode child = node.SampleChild();
			return WalkTree(child, is_player_one, iteration, player_one_reach_prob, player_two_reach_prob, with_sampling);
		}
		float val = 0;
		int child_index = 0;
		const std::vector<float> child_probabilities = node.ChildProbabilities();
		std::vector<SearchTreeNode> children = node.AllChildren();
		for (SearchTreeNode& child : children) {
			const float child_util = WalkTree(child, is_player_one, iteration, player_one_reach_prob, player_two_reach_prob, with_sampling);
			const float child_reach_prob = child_probabilities[child_index];
			val += child_reach_prob * child_util;
			child_index++;
		}
		return val;
	}
	else {
		float val = 0;
		const int num_children = node.NumChildren();
		std::vector<SearchTreeNode> children = node.AllChildren();
		std::vector<float> child_utilities(num_children, 0);
		InfoSetData info_set = InfoSetData(node.InfoSetPosition());
		for (int i_action = 0; i_action < num_children; i_action++)
		{
			float curr_strat_prob = info_set.GetCurrentStrategy(i_action);
			float child_utility;
			if (node.IsPlayerOne())
			{
				child_utility = WalkTree(children.at(i_action), is_player_one,
				                        iteration, curr_strat_prob * player_one_reach_prob,
				                        player_two_reach_prob, with_sampling);
			}
			else
			{
				child_utility = WalkTree(children.at(i_action), is_player_one,
				                        iteration, player_one_reach_prob,
				                        curr_strat_prob * player_two_reach_prob, with_sampling);
			}
			child_utilities.at(i_action) = child_utility;
			val += curr_strat_prob * child_utility;
		}
		if (node.IsPlayerOne() == is_player_one)
		{
			float regret_prob;
			float strat_prob;
			if (is_player_one)
			{
				regret_prob = player_two_reach_prob;
				strat_prob = player_one_reach_prob;
			}
			else 
			{
				regret_prob = player_one_reach_prob;
				strat_prob = player_two_reach_prob;
			}
			for (int i_action = 0; i_action < num_children; i_action++)
			{
				info_set.AddToCumulativeRegret(regret_prob * (child_utilities.at(i_action) - val), i_action);
				info_set.AddToCumulativeStrategy(strat_prob * info_set.GetCurrentStrategy(i_action), i_action);
			}
			RegretMatching(info_set);
		}
		return val;
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
RegretMatching(InfoSetData& info_set) {
	float regret_sum = 0;
	for (int i_action = 0; i_action < info_set.NumActions(); i_action++) 
	{
		float action_regret = info_set.GetCumulativeRegret(i_action);
		if (action_regret > 0)
		{
			info_set.SetCurrentStrategy(action_regret, i_action);
			regret_sum += action_regret;
		}
		else
		{
			info_set.SetCurrentStrategy(0.0, i_action);
		}
	}
	const float uniform_prob = 1.0 / static_cast<float>(info_set.NumActions());
	for (int i_action = 0; i_action < info_set.NumActions(); i_action++)
	{
		if (regret_sum > 0)
		{
			float curr_strat = info_set.GetCurrentStrategy(i_action);
			if (curr_strat < 0)
			{
				throw std::exception("invalid probability");
			}
			info_set.SetCurrentStrategy(curr_strat / regret_sum, i_action);
		}
		else
		{
			info_set.SetCurrentStrategy(uniform_prob, i_action);
		}
	}
}

template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	requires GenericCfrRequirements<Action, PlayerNode, ChanceNode, GameClass>
inline void CfrTree<Action, PlayerNode, ChanceNode, GameClass>::
AverageStrategy(const SearchTreeNode& node, std::unordered_set<Byte*> already_evaluated) {
	if (node.IsTerminalNode())
	{
		return;
	}
	else if (node.IsChanceNode())
	{
		std::vector<SearchTreeNode> children = node.AllChildren();
		for (auto& i_child : children)
		{
			AverageStrategy(i_child, already_evaluated);
		}
	}
	else {
		std::vector<SearchTreeNode> children = node.AllChildren();
		Byte* info_set_ptr = node.InfoSetPosition();
		if (!already_evaluated.contains(info_set_ptr)) {
			InfoSetData info_set = node.InfoSetPosition();
			float normalizing_sum = 0;
			for (int i_action = 0; i_action < info_set.NumActions(); i_action++)
			{
				normalizing_sum += info_set.GetCumulativeStrategy(i_action);
			}
			for (int i_action = 0; i_action < info_set.NumActions(); i_action++)
			{
				if (normalizing_sum > 0)
				{
					float curr_strat_sum = info_set.GetCumulativeStrategy(i_action);
					info_set.SetCurrentStrategy(curr_strat_sum / normalizing_sum, i_action);
				}
				else
				{
					info_set.SetCurrentStrategy(1.0f / static_cast<float>(info_set.NumActions()), i_action);
				}
			}
			already_evaluated.insert(info_set_ptr);
		}
	
		for (auto& i_child : children)
		{
			AverageStrategy(i_child, already_evaluated);
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
PrintTreeRecursive(const SearchTreeNode& node) {
	
	std::cout << node;
	if (node.IsPlayerNode() || node.IsChanceNode()) {
		
		std::vector<SearchTreeNode> children = node.AllChildren();
		for (SearchTreeNode& child_node : children) {
			PrintTreeRecursive(child_node);
		}
	}
	return;
}

