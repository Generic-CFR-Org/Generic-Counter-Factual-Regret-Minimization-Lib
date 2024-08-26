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
		ActionList firstStrat;
		ChanceNode() {
			prob = 1;
			Action* rock = new Action('r');;
			Action* paper = new Action('p');
			Action* scissors = new Action('s');
			firstStrat.push_back(rock);
			firstStrat.push_back(paper);
			firstStrat.push_back(scissors);

		}
		ChanceNode(int p) {
			prob = p; 
			Action* rock = new Action('r');;
			Action* paper = new Action('p');
			Action* scissors = new Action('s');
			firstStrat.push_back(rock);
			firstStrat.push_back(paper);
			firstStrat.push_back(scissors);
		}
		ChanceNode(const ChanceNode& other) {
			prob = other.prob;
			for (Action* a : other.firstStrat) {
				firstStrat.push_back(a);
			}
		}
	};


	typedef HistoryNode<GameState, ChanceNode, Action> GameHistoryNode;
	typedef std::vector<GameHistoryNode> History;


	using GameNodeChildren = ChildrenFromGameNode<RockPaperScissors::GameState, RockPaperScissors::ChanceNode, RockPaperScissors::Action>;
	using ChanceNodeChildren = ChildrenFromChanceNode<RockPaperScissors::GameState, RockPaperScissors::Action>;

	//Add static functions that determine whether a GameNode / ChanceNode is null. 
	static float TerminalRegret(History history) {
		
		auto iHistory = history.begin();
		auto iEnd = history.end();
		char player1;
		char player2;
		for (iHistory; iHistory < iEnd; iHistory++) {
			GameHistoryNode historyNode = *iHistory;
			if (historyNode.IsGameNode()) {
				GameState gameState = historyNode.GetGameState();
				Action* action = historyNode.GetAction();
				if (player1Action(gameState)) {
					player1 = action->action;
				}
				else {
					player2 = action->action;
				}
			}
		}

		if (player1 == 'r' && player2 == 's') {
			return 3.0;
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
	CFRGameTree<RockPaperScissors::GameState, RockPaperScissors::ChanceNode, RockPaperScissors::Action>* tree;

	RockPaperScissors() { tree = nullptr; }

	static bool player1Action(GameState game_state) {
		return game_state.player_with_action;
	}

	static GameNodeChildren* childrenFromGame(GameState gameState, ActionList strategy) {
		
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

	static ChanceNodeChildren* childrenFromChance(ChanceNode chance_node) {
		ChanceNodeChildren* pChildren = new ChanceNodeChildren();
		GameState start_state{ true };
		ActionList strat = chance_node.firstStrat;
		float prob = 1.0;
		pChildren->AddChildGameNode(start_state, strat, prob);

		return pChildren;
	}

	void CreateGameTree() {
		tree = new CFRGameTree<RockPaperScissors::GameState, RockPaperScissors::ChanceNode, RockPaperScissors::Action>(&childrenFromGame, &childrenFromChance, &player1Action, chanceNode, &TerminalRegret);
	}


};