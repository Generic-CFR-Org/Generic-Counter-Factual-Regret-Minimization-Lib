#include <iostream>
#include "RockPaperScissors.h"
#include "cfr.h"


int main(int argc, char* argv[]) {

	using Player = RockPaperScissors::Player;
	using ChanceNode = RockPaperScissors::ChanceNode;
	using Action = RockPaperScissors::Action;
	using Game = RockPaperScissors;

	using CFRTree = CfrTree<Action, Player, ChanceNode, Game>;

	RockPaperScissors* game = new RockPaperScissors();
	ChanceNode root = game->chanceNode;

	CFRTree* tree = new CFRTree(game, root);
	
	tree->ConstructTree();
	std::cout << "Before CFR: \n\n";
	tree->PrintTree();
	/*tree->PrintGameTree();*/

	/*tree->CFR(1000);

	std::cout << "After CFR: \n\n";
	tree->PrintGameTree();*/
}
