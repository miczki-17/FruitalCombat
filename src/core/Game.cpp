#include "Game.h"


namespace game
{
	Game::Game()
		: stateMachine(this)
	{
		window.create(sf::VideoMode({ config::WINDOW_WIDTH, config::WINDOW_LENGTH }), "Eco War");
		window.setFramerateLimit(config::FPS_LIMIT);

		stateMachine.changeState(states::StateType::Intro);
	}

	void Game::run()
	{
		sf::Clock clock;

		while (window.isOpen())
		{
			float dt = clock.restart().asSeconds();


			// EVENTS
			while (auto event = window.pollEvent())
			{
				const sf::Event& e = *event;

				if (e.is<sf::Event::Closed>())
					window.close();

				stateMachine.handleEvent(e);
			}

			// UPDATE
			stateMachine.update(dt);
			

			// RENDER
			window.clear();
			stateMachine.render(window);
			window.display();
		}
	}



	StateMachine& Game::getStateMachine()
	{
		return stateMachine;
	}

	sf::RenderWindow& Game::getWindow()
	{
		return window;
	}
}