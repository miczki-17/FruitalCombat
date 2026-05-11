#include <iostream>
#include "core/Game.h"
#include <memory>


int main() {
	std::unique_ptr game = std::make_unique<game::Game>();
	game->run();

	return 0;
}