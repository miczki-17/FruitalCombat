// --- Game.cpp ---


#include "Game.h"
#include "ResourceManager.h"
#include "AudioManager.h"

namespace game
{
	Game::Game()
		: stateMachine(this)
	{
		window.create(sf::VideoMode({ config::WINDOW_WIDTH, config::WINDOW_LENGTH }), "FRUITAL COMBAT");
		window.setFramerateLimit(config::FPS_LIMIT);

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

			// AUDIO update
			game::core::AudioManager::get().update();

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
		game::core::AudioManager::get().playSound("mouse_click");
	}


	//cursor
	void Game::drawMenuCursor()
	{
		auto& rm = game::core::ResourceManager::get();

		if (!rm.hasTexture("ui_cursor"))
			return;

		sf::Sprite cursor(*rm.getTexture("ui_cursor"));

		cursor.setOrigin({ 14.f, 10.f });

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			cursor.setTextureRect(sf::IntRect({ 0, 64 }, { 64, 64 }));
		else
			cursor.setTextureRect(sf::IntRect({ 0, 0 }, { 64, 64 }));

		sf::View oldView = window.getView();
		window.setView(window.getDefaultView());

		auto mousePos = sf::Mouse::getPosition(window);
		auto worldPos =
			window.mapPixelToCoords(mousePos, window.getDefaultView());

		cursor.setPosition(worldPos);

		window.draw(cursor);

		window.setView(oldView);
	}
}