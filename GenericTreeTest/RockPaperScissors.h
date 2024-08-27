#pragma once
#include <iostream>
#include "vector"
#include <utility>
#include <tuple>
#include <genericCFRGameTree.h>



class RockPaperScissors {
public:

	class Action {
	public:
		char action;
		Action() { action = 'n'; }
		inline Action(char in_action) { action = in_action; }
		Action(const Action& other) {action = other.action; }
		/*bool operator==(const Action& rhs) const noexcept {
			return this->action == rhs.action;
		}*/
	};	

	class GameState {
	public:
		bool player_with_action;

		GameState() { player_with_action = true; }
		GameState(bool p) { player_with_action = p; }
		GameState(const GameState& other) {player_with_action = other.player_with_action; }
		
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
	};

	typedef HistoryNode<GameState, ChanceNode, Action> GameHistoryNode;
	typedef std::vector<GameHistoryNode> History;

	using GameNodeChildren = ChildrenFromGameNode<RockPaperScissors::GameState, RockPaperScissors::ChanceNode, RockPaperScissors::Action>;
	using ChanceNodeChildren = ChildrenFromChanceNode<RockPaperScissors::GameState, RockPaperScissors::Action>;

	//Add static functions that determine whether a GameNode / ChanceNode is null. 
	static float TerminalRegret(History history, RockPaperScissors* gameInfo) {
		
		auto iHistory = history.begin();
		auto iEnd = history.end();
		char player1;
		char player2;
		for (iHistory; iHistory < iEnd; iHistory++) {
			GameHistoryNode historyNode = *iHistory;
			if (historyNode.IsGameNode()) {
				GameState gameState = historyNode.GetGameState();
				Action* action = historyNode.GetAction();
				if (player1Action(gameState, gameInfo)) {
					player1 = action->action;
				}
				else {
					player2 = action->action;
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
	Action *rock = new Action('r');;
	Action *paper = new Action('p');
	Action *scissors = new Action('s');
	ActionList strategy_profile{ rock, paper, scissors };
	CFRGameTree<RockPaperScissors::GameState, RockPaperScissors::ChanceNode, RockPaperScissors::Action, RockPaperScissors>* tree;

	RockPaperScissors() {}

	static bool player1Action(GameState game_state, RockPaperScissors* info) {
		return game_state.player_with_action;
	}

	static GameNodeChildren* childrenFromGame(GameState gameState, ActionList strategy, RockPaperScissors* info) {
		
		GameNodeChildren *pChildren = new GameNodeChildren();
		
		
		if (gameState.player_with_action) {
			GameState new_state{ false };
		
			pChildren->AddChildGameNode(new_state, strategy, strategy);
			return pChildren;

		}
		else {
			//Terminal node
			pChildren->AddChildTerminalNode(strategy);
			return pChildren;
		}	
	}

	static ChanceNodeChildren* childrenFromChance(ChanceNode chance_node, RockPaperScissors* info) {
		ChanceNodeChildren* pChildren = new ChanceNodeChildren();
		GameState start_state{ true };
		ActionList strat = info->strategy_profile;
		float prob = 1.0;
		pChildren->AddChildGameNode(start_state, strat, prob);

		return pChildren;
	}

};