#pragma once
#include "pch.h"
#include "framework.h"
#include <cstddef>
#include <concepts>
#include <functional>
#include <string>



/**
 * @brief Requirements for the client to use the generic cfr class.
 */
namespace CfrConcepts {

	/*
	- All Game classes must have ToHash() func that returns a string.
	 - PlayerNode must have ToInfoSetHash() func that returns a string for the player view.
	 */
	template<typename Action, typename PlayerNode, typename ChanceNode>
	concept Hashable = requires( const Action a, const PlayerNode p, const ChanceNode c ) {
		/*All Game classes must have ToHash() func that returns a string.*/
		{ a.ToHash() } -> std::convertible_to<std::string>;
		{ p.ToHash() } -> std::convertible_to<std::string>;
		{ c.ToHash() } -> std::convertible_to<std::string>;

		{ p.ToInfoSetHash() } -> std::convertible_to<std::string>;
	};

	/*
	- PlayerNode must have function IsPlayerOne() that returns a bool
	for the currently acting player
	*/
	template<typename PlayerNode>
	concept PlayerNodePlayerOneFunc = requires( const PlayerNode p ) {

		{ p.IsPlayerOne() } -> std::convertible_to<bool>;

	};
}

/**
 * @brief Node used by the client to store the child of a player node or chance node.
 *	      Can store either a player node, chance node, or no node, alongside
 *		  metadata describing the action taken to get to a node such as:
 *			float - probability to reach a node from a parent chance node.
 *			action - action taken to reach a node from a parent player node.
 */
template<typename Action, typename PlayerNode, typename ChanceNode>
class ClientNode {
public:
	Action action_;
	PlayerNode player_node_;
	ChanceNode chance_node_;

	float probability_;

	bool is_player_node_;
	bool is_chance_node_;
	bool is_terminal_node_;

	ClientNode(PlayerNode p) :
		action_{}, player_node_{ p }, chance_node_{}, probability_{ 1.0 },
		is_player_node_{ true }, is_chance_node_{ false },
		is_terminal_node_{ false } {}

	ClientNode(ChanceNode c) :
		action_{}, player_node_{}, chance_node_{ c }, probability_{ 1.0 },
		is_player_node_{ false }, is_chance_node_{ true },
		is_terminal_node_{ false } {}

	ClientNode() :
		action_{}, player_node_{}, chance_node_{}, probability_{ 1.0 },
		is_player_node_{ false }, is_chance_node_{ false },
		is_terminal_node_{ true } {}

	ClientNode(PlayerNode p, float prob) :
		ClientNode{ p } { probability_ = prob; }

	ClientNode(ChanceNode c, float prob) :
		ClientNode{ c } {probability_ = prob; }

	ClientNode(float prob) :
		ClientNode{} {probability_ = prob; } 

	ClientNode(PlayerNode p, Action a) :
		ClientNode{ p } { action_ = a; }

	ClientNode(ChanceNode c, Action a) :
		ClientNode{ c } { action_ = a;}

	ClientNode(Action a) :
		ClientNode{} {
		action_ = a;
	}

	bool IsPlayerNode() const { return is_player_node_; }
	bool IsChanceNode() const { return is_chance_node_; }
	bool IsTerminalNode() const { return is_terminal_node_; }

	Action GetAction() { return action_; }
	PlayerNode GetPlayerNode() { return player_node_; }
	ChanceNode GetChanceNode() { return chance_node_; }
	float GetProbability() const { return probability_; }
};


template<typename Action, typename PlayerNode, typename ChanceNode>
requires CfrConcepts::Hashable<Action, PlayerNode, ChanceNode>&&
		 CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>
class TreeNode {

	using byte = unsigned char;
	using TreeNodeList = std::vector<TreeNode>;
protected:
	Action action_;
	PlayerNode player_node_;
	ChanceNode chance_node_;

	float probability_;

	bool is_player_node_;
	bool is_chance_node_;
	bool is_terminal_node_;
	TreeNode* parent_;
	byte* child_start_pos_;
	bool is_child_start_set_;

public:

	TreeNode(const TreeNode<Action, PlayerNode, ChanceNode>& other) {
		action_ = other.action_;
		player_node_ = other.player_node_;
		chance_node_ = other.chance_node_;
		probability_ = other.probability_;
		is_player_node_ = other.is_player_node_;
		is_chance_node_ = other.is_chance_node_;
		is_terminal_node_ = other.is_terminal_node_;
		parent_ = other.parent_;
		child_start_pos_ = nullptr;
		is_child_start_set_ = false;
	}

	TreeNode (
		const TreeNode<Action, PlayerNode, ChanceNode>& other,
		TreeNode<Action, PlayerNode, ChanceNode>* parent
	) {
		action_ = other.action_;
		player_node_ = other.player_node_;
		chance_node_ = other.chance_node_;
		probability_ = other.probability_;
		is_player_node_ = other.is_player_node_;
		is_chance_node_ = other.is_chance_node_;
		is_terminal_node_ = other.is_terminal_node_;
		parent_ = parent;
		child_start_pos_ = nullptr;
		is_child_start_set_ = false;
	}

	TreeNode(
		const ClientNode<Action, PlayerNode, ChanceNode>& other
	) {
		action_ = other.action_;
		player_node_ = other.player_node_;
		chance_node_ = other.chance_node_;
		probability_ = other.probability_;
		is_player_node_ = other.is_player_node_;
		is_chance_node_ = other.is_chance_node_;
		is_terminal_node_ = other.is_terminal_node_;
		parent_ = nullptr;
		child_start_pos_ = nullptr;
		is_child_start_set_ = false;
	}

	TreeNode(
		const ClientNode<Action, PlayerNode, ChanceNode>& other,
		TreeNode<Action, PlayerNode, ChanceNode>* parent
	) {
		action_ = other.action_;
		player_node_ = other.player_node_;
		chance_node_ = other.chance_node_;
		probability_ = other.probability_;
		is_player_node_ = other.is_player_node_;
		is_chance_node_ = other.is_chance_node_;
		is_terminal_node_ = other.is_terminal_node_;
		parent_ = parent;
		child_start_pos_ = nullptr;
		is_child_start_set_ = false;
	}



	/**
		* @brief Base constructors for each node type. 
		*/
	TreeNode(PlayerNode p) :
		action_{}, player_node_{ p }, chance_node_{}, probability_{ 1.0 },
		is_player_node_{ true }, is_chance_node_{ false },
		is_terminal_node_{ false }, parent_{ nullptr },
		child_start_pos_{ nullptr }, is_child_start_set_{ false } {}

	TreeNode(ChanceNode c) :
		action_{}, player_node_{}, chance_node_{ c }, probability_{ 1.0 },
		is_player_node_{ false }, is_chance_node_{ true },
		is_terminal_node_{ false }, parent_{ nullptr },
		child_start_pos_{nullptr}, is_child_start_set_{ false } {}

	TreeNode() :
		action_{}, player_node_{}, chance_node_{}, probability_{ 1.0 },
		is_player_node_{ false }, is_chance_node_{ false },
		is_terminal_node_{ true }, parent_{ nullptr },
		child_start_pos_{nullptr}, is_child_start_set_{ false } {}

	/**
		* @brief Constructors that give node a parent tree node.
		* @param parent Pointer to parent Tree Node.
		*/
	TreeNode(PlayerNode p, TreeNode* parent) : 
		TreeNode{ p }, parent_{ parent } {}

	TreeNode(ChanceNode c, TreeNode* parent) :
		TreeNode{ c }, parent_{ parent } {}

	TreeNode(TreeNode* parent) :
		TreeNode{}, parent_{ parent } {}


	/**
		* @brief Constructors that set action taken to get to the node.
		* @param a Action taken by parent node.
		*/
	TreeNode(PlayerNode p, Action a) :
		TreeNode{ p }, action_{ a } {}

	TreeNode(ChanceNode c, Action a) :
		TreeNode{ c }, action_{ a } {}

	TreeNode(Action a) : TreeNode{}, action_{ a } {}

	/**
		* @brief Constructors that set probability to get to the node.
		* @param prob Probability of reach node.
		*/
	TreeNode(PlayerNode p, float prob) :
		TreeNode{ p }, probability_{ prob } {}

	TreeNode(ChanceNode c, float prob) :
		TreeNode{ c }, probability_{ prob } {}

	TreeNode(float prob) :
		TreeNode{}, probability_{ prob } {}


	/**
	 * @brief Returns the type of node stored in the current tree node.
	 */
	bool IsPlayerNode() { return is_player_node_; }
	bool IsChanceNode() { return is_chance_node_; }
	bool IsTerminalNode() { return is_terminal_node_; }

	/**
	 * @brief Returns whether a tree node's child has updated the parent's child pointer.
	 */
	bool IsChildOffsetSet() { return is_child_start_set_; }

	/**
	 * @brief Gettors for elements stored in a tree node.
	 */
	Action GetAction() { return action_; }
	PlayerNode GetPlayerNode() { return player_node_; }
	ChanceNode GetChanceNode() { return chance_node_; }
	float GetProbability() { return probability_; }
	byte* GetChildOffset() { return child_start_pos_; }

	/**
	 * @return Gets string representation of the history for info set evaluation.
	 */
	std::string HistoryHash() {
		if (!this->IsPlayerNode()) {
			return "";
		}
		bool isPlayerOne = this->GetPlayerNode().IsPlayerOne();
		return HistoryHashRecursive(isPlayerOne);
	}

	/**
	 * @brief Gets the list representation of the history up until the current node.
	 * @return List of TreeNodes.
	 */
	TreeNodeList HistoryList() {
		TreeNodeList historyList;
		HistoryListRecursive(historyList, nullptr);
		return historyList;
	}

	/**
	 * @brief Updates a node's parent's child start offset for search tree navigation.
	 * @param childOffset Address of the current node in the search tree.
	 */
	void UpdateParentOffset(byte* childOffset) {
		if (this->parent_ == nullptr) {
			return;
		}
		if (this->parent_->IsChildOffsetSet()) {
			return;
		}
		this->parent_->child_start_pos_ = childOffset;
		this->parent_->is_child_start_set_ = true;
	}


private:

	std::string HistoryHashRecursive(bool isPlayerOne) {
		if (parent_ == nullptr) {
			return "";
		} else if (this->IsChanceNode()) {
			return this->GetChanceNode().ToHash() + parent_->HistoryHash();
		} else if (this->GetPlayerNode().IsPlayerOne() == isPlayerOne) {
			std::string currentHash = this->GetPlayerNode().ToInfoSetHash();
			currentHash += this->GetAction().ToHash();
			return currentHash + parent_->HistoryHashRecursive(isPlayerOne);
		} else {
			return this->GetAction().ToHash() + parent_->HistoryHashRecursive(isPlayerOne);
		}	
	}

	void HistoryListRecursive(TreeNodeList& historyList, TreeNode* lastChild) {
		if (this == nullptr) {
			return;
		}
		else if (this->parent_ == nullptr) {
			historyList.push_back(*this);
		}
		else {
			this->parent_->HistoryListRecursive(historyList, this);
			TreeNode toAdd(*this);
			if (lastChild != nullptr) {
				toAdd.action_ = lastChild->GetAction();
			}
			else {
				toAdd.action_ = Action();
			}
			historyList.push_back(toAdd);
		}
	}
};

/**
 * @return List of probabilities generated for the children of a chance node.
 */
template<typename Action, typename PlayerNode, typename ChanceNode>
static std::vector<float> ToFloatList
(
	std::vector<ClientNode<Action, PlayerNode, ChanceNode>>& nodeList
) {
	std::vector<float> floatList;
	for (ClientNode<Action, PlayerNode, ChanceNode> node : nodeList) {
		floatList.push_back(node.GetProbability());
	}
	return floatList;
}

/**
 * @brief Requirements for the client to be able to use the generic cfr class.
 */
namespace CfrConcepts {

	/*Player Node must have function that returns child node of an action.*/
	template<typename Action, typename PlayerNode, typename ChanceNode, typename GameState>
	concept PlayerNodeChildFunc = requires( const Action a, const PlayerNode p, const GameState g ) {
		{ p.Child(a, &g) } -> std::convertible_to<ClientNode<Action, PlayerNode, ChanceNode>>;

	};

	/*Player node must have function that returns a list of actions it can take*/
	template<typename Action, typename PlayerNode, typename GameState>
	concept PlayerNodeActionListFunc = requires ( const PlayerNode p, const GameState g ) {
		{ p.ActionList(&g) } -> std::convertible_to<std::vector<Action>>;
	};


	/*Chance Node must have function that returns vector of child nodes*/
	template<typename Action, typename PlayerNode, typename ChanceNode, typename GameState>
	concept ChanceNodeChildrenFunc = requires( const Action a, const PlayerNode p, const ChanceNode c, const GameState g ) {
		{ c.Children(&g) } -> std::convertible_to<std::vector<ClientNode<Action, PlayerNode, ChanceNode>>>;

	};

	/*Client Game class must have a function that evaluates the utility at a terminal node.*/
	template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	concept NeedsUtilityFunc = requires( const Action a, const PlayerNode p, const ChanceNode c, const GameClass g ) {
		
		{ g.UtilityFunc(std::vector<TreeNode<Action, PlayerNode, ChanceNode>>()) } ->
			std::convertible_to<float>;
	};

}


















