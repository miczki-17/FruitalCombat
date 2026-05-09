#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


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


		std::vector<sf::Image> menuImageBuffer;
	};
}