#include <iostream>
#include "RockPaperScissors.h"


int main(int argc, char* argv[]) {

	using GameState = RockPaperScissors::GameState;
	using ChanceNode = RockPaperScissors::ChanceNode;
	using Action = RockPaperScissors::Action;
	using Game = RockPaperScissors;

	using CFRTree = CFRGameTree<GameState, ChanceNode, Action, Game>;

	RockPaperScissors* game = new RockPaperScissors();

	CFRTree* tree = new CFRTree(game, &Game::childrenFromGame, &Game::childrenFromChance, &Game::player1Action, game->chanceNode, &Game::TerminalRegret);
	
	tree->ConstructTree();
	std::cout << "Before CFR: \n\n";

	tree->CFR(5000);

	std::cout << "After CFR: \n\n";
	tree->PrintGameTree();
}
