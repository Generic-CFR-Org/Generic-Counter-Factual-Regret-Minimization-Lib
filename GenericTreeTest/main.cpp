#include <iostream>
#include "RockPaperScissors.h"


int main(int argc, char* argv[]) {

	RockPaperScissors* game = new RockPaperScissors();
	game->CreateGameTree();
	
	game->tree->ConstructTree();
	
	game->tree->CFR(300, 0);

	std::cout << "After CFR: \n\n";
	game->tree->PrintGameTree();
}
