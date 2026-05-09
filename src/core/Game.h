#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <map>
#include <string>
#include "Config.h"
#include "StateMachine.h"




namespace game
{
	class Game
	{
	private:
		sf::RenderWindow window;
		StateMachine stateMachine;


	public:
		Game();

		void run();

		StateMachine& getStateMachine();
		sf::RenderWindow& getWindow();


		// pre-loading data buffers
		std::vector<sf::Image> menuImageBuffer;
		std::map<std::string, sf::Image> menuUiBuffer;
	};
}