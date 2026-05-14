#include "Game.h"

namespace game
{
	Game::Game()
		: stateMachine(this)
	{
		window.create(sf::VideoMode({ config::WINDOW_WIDTH, config::WINDOW_LENGTH }), "FRUITAL COMBAT");
		window.setFramerateLimit(config::FPS_LIMIT);

		// load global sounds
		if (!uiClickBuffer.loadFromFile("../../../assets/audio/ui/click.mp3"))
		{
			std::cerr << "can not load uiClickBuffer\n";
		}
		uiClickSound.emplace(uiClickBuffer);

		stateMachine.changeState(states::StateType::Intro);
	}

	void Game::run()
	{
		sf::Clock clock;

		while (window.isOpen())
		{
			// 1. FSM change states
			stateMachine.processStateChanges();

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

	// UI helpers
	void Game::playUIClick()
	{
		uiClickSound->stop();
		uiClickSound->play();
	}
}