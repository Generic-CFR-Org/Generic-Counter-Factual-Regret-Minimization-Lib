#include <iostream>
#include "RockPaperScissors.h"


int main(int argc, char* argv[]) {

	RockPaperScissors* game = new RockPaperScissors();
	game->CreateGameTree();
	int result = game->tree->PreProcessor();
	std::cout << result << "\n";
	int action_size = sizeof(RockPaperScissors::Action);
	int game_state_size = sizeof(RockPaperScissors::GameState);
	int terminal_size = sizeof(RockPaperScissors::TerminalNode);
	int chance_size = sizeof(RockPaperScissors::ChanceNode);
	int common_size = sizeof(RockPaperScissors::CommonGameState);
	int strat_size = game->strategy_profile->size() * 2 * (int) sizeof(float);
	std::cout << action_size << "\n";
	std::cout << game_state_size << "\n";
	std::cout << terminal_size << "\n";
	std::cout << chance_size << "\n";
	std::cout << common_size << "\n";
	std::cout << strat_size << "\n";
	
	game->tree->ConstructTree();
	game->tree->PrintGameTree();
	game->tree->CFR(500, 0);
	/*for (int i = 0; i < result; i+=sizeof(float)) {
		std::cout << (float) game->tree->gameTree[i];
	}*/
	game->tree->PrintGameTree();
}
