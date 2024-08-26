#include <iostream>
#include "RockPaperScissors.h"


int main(int argc, char* argv[]) {

	RockPaperScissors* game = new RockPaperScissors();
	game->CreateGameTree();
	int result = game->tree->PreProcessor();
	std::cout << result << "\n";

	
	game->tree->ConstructTree();

	std::cout << "Before CFR: \n\n";
	game->tree->PrintGameTree();
	
	game->tree->CFR(50, 0);

	std::cout << "After CFR: \n\n";
	game->tree->PrintGameTree();
}
