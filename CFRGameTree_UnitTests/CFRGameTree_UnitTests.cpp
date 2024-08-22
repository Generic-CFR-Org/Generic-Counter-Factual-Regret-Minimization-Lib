#include "pch.h"
#include "CppUnitTest.h"
#include "..\GenericCFRGameTree\genericCFRGameTree.h"
#include "iostream"


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

	class ChanceNode {
	public:
		int prob;
		ChanceNode() { prob = 1; }
	};

	class TerminalNode {
	public:
		char player_1_choice;
		char player_2_choice;
		TerminalNode(char player1, char player2) {
			player_1_choice = player1;
			player_2_choice = player2;
		}
	};

	class CommonGameState {};

	typedef std::vector<Action*> StrategyProfile;
	typedef std::tuple<GameState*, StrategyProfile*> GameNode;
	typedef std::tuple<GameState*, StrategyProfile*, float> GameNodeWithProb;
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

	RockPaperScissors() {tree = nullptr;}

	static std::tuple<GameNode*, TerminalNode*, ChanceNode*>* childFromGame(GameState* player, Action* action, CommonGameState* common, StrategyProfileList* all_strategies) {
		ChildNode* child_node;
		if (player->player_with_action = 1) {
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
		float prob = 1.0;
		GameNodeWithProb* startNode = new GameNodeWithProb{ start_state, new_strat, prob };
		GameNodeListFromChance* startNodeList = new GameNodeListFromChance{ startNode };
		TerminalNodeListFromChance* emptyTerminalList = new TerminalNodeListFromChance{};
		ChildrenFromChance* children_FromChance = new ChildrenFromChance{ startNodeList, emptyTerminalList };
		return children_FromChance;
	}

	/*void CreateGameTree() {
		tree = new CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
			(no_common, &childFromGame, &childrenFromChance, chanceNode, strat_list);
	}*/


};

namespace CFRGameTreeUnitTests
{
	TEST_CLASS(CFRGameTreeUnitTests)
	{
	public:

		TEST_METHOD(RockPaperScissorsTest)
		{
			

		}
	};
}



