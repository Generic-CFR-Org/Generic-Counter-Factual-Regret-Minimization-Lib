#pragma once
#include <iostream>
#include "vector"
#include <utility>
#include <tuple>
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
		char action;
		Action() { action = 'n'; }
		inline Action(char in_action) { action = in_action; }
		/*Action(const Action& other) {action = other.action; }*/
		std::string ToHash() { return std::string(1, ' '); }
	};	

	class Player {
	public:
		bool player_with_action;

		Player() { player_with_action = true; }
		Player(bool p) { player_with_action = p; }
		Player(const Player& other) {player_with_action = other.player_with_action; }
		bool IsPlayerOne() { return player_with_action; }
		std::string ToHash() { return std::string(1, player_with_action); }
		std::string ToInfoSetHash() { return std::string(1, player_with_action); }
		Node Child(Action a, RockPaperScissors* gameInfo) {
			if (player_with_action) {
				Node childNode{Player{false}, a};
				return childNode;
			}
			else {
				Node terminalNode{a};
				return terminalNode;
			}
		}
		std::vector<Action> ActionList(RockPaperScissors* gameInfo) {
			Action rock{'r'};
			Action paper{'p'};
			Action scissors{'s'};
			std::vector<Action> allActions;
			allActions.push_back(rock);
			allActions.push_back(paper);
			allActions.push_back(scissors);
			return allActions;
		}
	};

	typedef std::vector<Action*> ActionList;

	class ChanceNode {
	public:
		int prob;
		ChanceNode() {
			prob = 1;

		}
		ChanceNode(int p) {
			prob = p; 
		}
		ChanceNode(const ChanceNode& other) {
			prob = other.prob;
		}
		std::string ToHash() { return std::string(1, ' '); }
		std::vector<Node> Children(RockPaperScissors* gameInfo) {
			std::vector<Node> startList;
			Player playerOne{true};
			Node playerNode{playerOne, 1.0};
			startList.push_back(playerNode);
			return startList;
		}
	};

	using HistoryNode = TreeNode<Action, Player, ChanceNode>;

	float UtilityFunc(std::vector<HistoryNode> history) {

		auto iHistory = history.begin();
		auto iEnd = history.end();
		char player1 = 'a';
		char player2 = 'a';
		for (iHistory; iHistory < iEnd; iHistory++) {
			HistoryNode historyNode = *iHistory;
			if (historyNode.IsPlayerNode()) {
				Player playerNode = historyNode.GetPlayerNode();
				Action action = historyNode.GetAction();
				if (playerNode.IsPlayerOne()) {
					player1 = action.action;
				}
				else {
					player2 = action.action;
				}
			}
		}

		if (player1 == 'r' && player2 == 's') {
			return 1.0;
		}
		if (player1 == 'r' && player2 == 'p') {
			return -1.0;
		}
		if (player1 == 'p' && player2 == 'r') {
			return 1.0;
		}
		if (player1 == 'p' && player2 == 's') {
			return -1.0;
		}
		if (player1 == 's' && player2 == 'p') {
			return 1.0;
		}
		if (player1 == 's' && player2 == 'r') {
			return -1.0;
		}
		return 0.0;
	}

	ChanceNode chanceNode{};

	RockPaperScissors() {}

};