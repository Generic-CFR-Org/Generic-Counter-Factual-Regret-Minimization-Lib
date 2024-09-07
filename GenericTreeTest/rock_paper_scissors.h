#pragma once
#include "vector"
#include "cfr.h"
#include "nodes.h"

class RockPaperScissors {
public:

	class Action;
	class Player;
	class ChanceNode;

	using Node = ClientNode<Action, Player, ChanceNode>;

	class Action {
	public:
		char action_;
		Action() { action_ = 'n'; }
		inline Action(char in_action) { action_ = in_action; }
		/*Action(const Action& other) {action = other.action; }*/
		std::string ToHash() const { return std::string(1, ' '); }
	};	

	class Player {
	public:
		bool player_with_action_;

		Player() { player_with_action_ = true; }
		explicit Player(bool p) { player_with_action_ = p; }
		Player(const Player& other) {player_with_action_ = other.player_with_action_; }
		bool IsPlayerOne() const { return player_with_action_; }
		std::string ToHash() const { return "PNode"; }
		std::string ToInfoSetHash() const
		{
			return player_with_action_ ? "One" : "Two";
		}
		Node Child(const Action a, const RockPaperScissors* game_info) const
		{
			if (player_with_action_) {
				Node childNode{Player{false}, a};
				return childNode;
			}
			else {
				Node terminalNode{a};
				return terminalNode;
			}
		}

		std::vector<Action> ActionList(const RockPaperScissors* game_info) const {
			const Action rock{'r'};
			const Action paper{'p'};
			const Action scissors{'s'};
			std::vector<Action> all_actions;
			all_actions.push_back(rock);
			all_actions.push_back(paper);
			all_actions.push_back(scissors);
			return all_actions;
		}
	};

	typedef std::vector<Action*> ActionList;

	class ChanceNode {
	public:
		int prob;
		ChanceNode() {
			prob = 1;

		}

		explicit ChanceNode(int p) {
			prob = p; 
		}
		ChanceNode(const ChanceNode& other) {
			prob = other.prob;
		}

		std::string ToHash() const { return std::string(1, ' '); }
		std::vector<Node> Children(const RockPaperScissors* game_info) const {
			std::vector<Node> start_list;
			const Player player_one{true};
			const Node playerNode{player_one, 1.0};
			start_list.push_back(playerNode);
			return start_list;
		}
	};

	using HistoryNode = TreeNode<Action, Player, ChanceNode>;

	float UtilityFunc(std::vector<HistoryNode> history) const {

		auto i_history = history.begin();
		auto i_end = history.end();
		char player_one = 'a';
		char player_two = 'a';
		for (i_history; i_history < i_end; i_history++) {
			HistoryNode history_node = *i_history;
			if (history_node.IsPlayerNode()) {
				Player playerNode = history_node.GetPlayerNode();
				Action action = history_node.GetAction();
				if (playerNode.IsPlayerOne()) {
					player_one = action.action_;
				}
				else {
					player_two = action.action_;
				}
			}
		}

		if (player_one == 'r' && player_two == 's') {
			return 1.0;
		}
		if (player_one == 'r' && player_two == 'p') {
			return -1.0;
		}
		if (player_one == 'p' && player_two == 'r') {
			return 1.0;
		}
		if (player_one == 'p' && player_two == 's') {
			return -1.0;
		}
		if (player_one == 's' && player_two == 'p') {
			return 2.0;
		}
		if (player_one == 's' && player_two == 'r') {
			return -1.0;
		}
		return 0.0;
	}

	ChanceNode chance_node_{};

	RockPaperScissors() = default;
};