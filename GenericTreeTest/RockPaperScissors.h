#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <tuple>
#include "genericCFRGameTree.h"


class RockPaperScissors {
public:

	class Action {
	public:
		char action;
		Action() { action = 'n'; }
		inline Action(char in_action) { action = in_action; }
	};
	class Rock : Action { Rock() { action = 'r'; } };
	class Paper : Action { Paper() { action = 'p'; } };
	class Scissors : Action { Scissors() { action = 's'; } };


	class GameState {
	public:
		bool player_with_action;
		char player_1_action;
		char player_2_action;
		GameState() { player_with_action = true; player_1_action = 'n'; player_2_action = 'n'; }
		GameState(bool p) { player_with_action = p; player_1_action = 'n'; player_2_action = 'n'; }
		GameState(bool p, char player_1_choice, char player_2_choice) {
			player_with_action = p;
			player_1_action = player_1_choice;
			player_2_action = player_2_choice;
		}
	};
	class Player1 : GameState { Player1() { player_with_action = true; } };
	class Player2 : GameState { Player2() { player_with_action = false; } };

	static int serializeGameState(GameState* game_state, unsigned char* buf) {
		buf[0] = game_state->player_with_action;
		buf[1] = game_state->player_1_action;
		buf[2] = game_state->player_2_action;
		return 3;
	}

	static GameState* deserializeGameState(unsigned char* buf, int size) {
		GameState* deserialized_gamestate = new GameState((bool) buf[0], (char) buf[1], (char) buf[2]);
		return deserialized_gamestate;
	}

	class ChanceNode {
	public:
		int prob;
		ChanceNode() { prob = 1; }
		ChanceNode(int p) {prob = p; }
	};

	static int serializeChanceNode(ChanceNode* chance_node, unsigned char* buf) {
		buf[0] = chance_node->prob;
		return sizeof(chance_node->prob);
	}

	static ChanceNode* deserializeChanceNode(unsigned char* buf, int size) {
		ChanceNode* deserialized_chance_node = new ChanceNode((int) buf[0]);
		return deserialized_chance_node;
	}

	class TerminalNode {
	public:
		char player_1_choice;
		char player_2_choice;
		TerminalNode(char player1, char player2) {
			player_1_choice = player1;
			player_2_choice = player2;
		}
	};

	static int serializeTerminal(TerminalNode* terminal_node, unsigned char* buf) {
		buf[0] = terminal_node->player_1_choice;
		buf[1] = terminal_node->player_2_choice;
		return 2 * sizeof(char);
	}

	static TerminalNode* deserializeTerminal(unsigned char* buf, int size) {
		TerminalNode* new_terminal_node = new TerminalNode((char) buf[0], (char) buf[1]);
		return new_terminal_node;
	}

	static float TerminalRegret(TerminalNode* terminal_node) {
		char player1 = terminal_node->player_1_choice;
		char player2 = terminal_node->player_2_choice;
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

	class CommonGameState {};

	typedef std::vector<Action*> StrategyProfile;
	typedef std::tuple<GameState*, StrategyProfile*> GameNode;
	typedef std::tuple<GameNode*, float> GameNodeWithProb;
	typedef std::tuple<TerminalNode*, float> TerminalNodeWithProb;
	typedef std::vector<StrategyProfile*> StrategyProfileList;
	typedef std::vector<GameNode*> GameNodeList;
	typedef std::vector<TerminalNode*> TerminalNodeList;
	typedef std::vector<ChanceNode*> ChanceNodeList;

	typedef std::vector<GameNodeWithProb*> GameNodeListFromChance;
	typedef std::vector<TerminalNodeWithProb*> TerminalNodeListFromChance;

	typedef std::tuple<GameNode*, TerminalNode*, ChanceNode*> ChildNode;
	typedef std::tuple<GameNodeListFromChance*, TerminalNodeListFromChance*> ChildrenFromChance;


	/*static constexpr CommonGameState* no_common = nullptr;
	static constexpr ChanceNode* chanceNode = new ChanceNode();
	static constexpr Action* rock = new Action('r');
	static constexpr Action* paper = new Action('p');
	static constexpr Action* scissors = new Action('s');
	static constexpr StrategyProfile* strategy_profile = new StrategyProfile{ rock, paper, scissors };
	static constexpr StrategyProfileList* strat_list = new StrategyProfileList{ strategy_profile };*/

	CommonGameState* no_common = nullptr;
	ChanceNode* chanceNode = new ChanceNode();
	Action* rock = new Action('r');
	Action* paper = new Action('p');
	Action* scissors = new Action('s');
	StrategyProfile* strategy_profile = new StrategyProfile{ rock, paper, scissors };
	StrategyProfileList* strat_list = new StrategyProfileList{ strategy_profile };
	CFRGameTree<RockPaperScissors::Action, RockPaperScissors::CommonGameState, RockPaperScissors::GameState, RockPaperScissors::TerminalNode, RockPaperScissors::ChanceNode>* tree;

	RockPaperScissors() { tree = nullptr; }

	static bool player1Action(GameState* game_state) {
		return game_state->player_with_action;
	}

	static std::tuple<GameNode*, TerminalNode*, ChanceNode*>* childFromGame(GameState* player, Action* action, CommonGameState* common, StrategyProfileList* all_strategies) {
		ChildNode* child_node;
		if (player->player_with_action) {
			GameState* new_state;
			if (action->action == 'r') {
				new_state = new GameState(false, 'r', 'n');
			}
			if (action->action == 'p') {
				new_state = new GameState(false, 'r', 'n');
			}
			else {
				new_state = new GameState(false, 'r', 'n');
			}
			StrategyProfile* new_strat = all_strategies->at(0);
			GameNode* new_node = new GameNode{ new_state, new_strat };
			child_node = new ChildNode{ new_node, nullptr, nullptr };

		}
		else {
			TerminalNode* new_terminal;
			if (action->action == 'r') {
				new_terminal = new TerminalNode(player->player_1_action, 'r');
			}
			if (action->action == 'p') {
				new_terminal = new TerminalNode(player->player_1_action, 'p');
			}
			else {
				new_terminal = new TerminalNode(player->player_1_action, 's');
			}
			child_node = new ChildNode{ nullptr, new_terminal, nullptr };
		}
		return child_node;
	}

	static ChildrenFromChance* childrenFromChance(ChanceNode* chance_node, CommonGameState* common, StrategyProfileList* all_strategies) {
		GameState* start_state = new GameState(true);
		StrategyProfile* new_strat = all_strategies->at(0);
		GameNode* new_game_node = new GameNode{start_state, new_strat};
		float prob = 1.0;
		GameNodeWithProb* startNode = new GameNodeWithProb{ new_game_node, prob };
		GameNodeListFromChance* startNodeList = new GameNodeListFromChance{ startNode };
		TerminalNodeListFromChance* emptyTerminalList = new TerminalNodeListFromChance{};
		ChildrenFromChance* children_FromChance = new ChildrenFromChance{ startNodeList, emptyTerminalList };
		return children_FromChance;
	}

	void CreateGameTree() {
		tree = new CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
			(no_common, &childFromGame, &childrenFromChance, &player1Action,
			 chanceNode, strat_list, &TerminalRegret);
	}


};