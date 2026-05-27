// --- Game.cpp ---


#include "Game.h"
#include "ResourceManager.h"

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


	//cursor
	void Game::drawMenuCursor()
	{
		if (!isCursorInitialized)
		{
			auto& rm = game::core::ResourceManager::get();

			if (rm.hasTexture("ui_cursor"))
			{
				menuCursorSprite.emplace(
					*rm.getTexture("ui_cursor"));

				menuCursorSprite->setTextureRect(
					sf::IntRect({ 0, 0 }, { 64, 64 }));

				menuCursorSprite->setOrigin({
					14.f, 10.f
					});

				window.setMouseCursorVisible(false);
				isCursorInitialized = true;
			}
		}

		if (isCursorInitialized &&
			menuCursorSprite.has_value())
		{
			sf::View oldView = window.getView();
			window.setView(window.getDefaultView());

			sf::Vector2i mousePos =
				sf::Mouse::getPosition(window);

			sf::Vector2f worldPos =
				window.mapPixelToCoords(
					mousePos,
					window.getDefaultView());

			menuCursorSprite->setPosition(worldPos);

			if (sf::Mouse::isButtonPressed(
				sf::Mouse::Button::Left))
			{
				menuCursorSprite->setTextureRect(
					sf::IntRect({ 0, 64 }, { 64, 64 }));
			}
			else
			{
				menuCursorSprite->setTextureRect(
					sf::IntRect({ 0, 0 }, { 64, 64 }));
			}

			window.draw(*menuCursorSprite);
			window.setView(oldView);
		}
	}
}