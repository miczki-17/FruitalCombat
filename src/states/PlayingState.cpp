#include "PlayingState.h"
#include "../core/Game.h"
#include <iostream>

namespace game::states
{
	PlayingState::PlayingState(game::Game* game)
		: State(game)
	{
		std::cout << "[PLAYING STATE] Budowanie areny d¿ungli...\n";

		// 1. £ADOWANIE MAPY PNG
		if (mapTexture.loadFromFile("../../../assets/textures/maps/jungle_arena.png"))
		{
			mapSprite.emplace(mapTexture);


			sf::Vector2u size = mapTexture.getSize();
			mapLimits = sf::Vector2f(static_cast<float>(size.x), static_cast<float>(size.y));

			std::cout << "[MAPA] Za³adowano pomyœlnie. Wymiary: " << mapLimits.x << "x" << mapLimits.y << "\n";
		}
		else
		{
			std::cerr << "[B£¥D] Nie mo¿na za³adowaæ pliku jungle_arena.png!\n";
			mapLimits = sf::Vector2f(4000.f, 4000.f); // Awaryjne granice
		}

		// 2. ALOKACJA GRACZA
		player = std::make_unique<game::entities::Player>();

		if (player != nullptr)
		{
			// Odczyt domyœlnego wyboru postaci
			player->initFruit(game::entities::FruitType::Apple);

			// Ustawienie gracza idealnie w centrum wczytanej mapy
			player->setPosition({ mapLimits.x / 2.0f, mapLimits.y / 2.0f });
		}

		sf::View view = game->getWindow().getView();
		view.zoom(1.25f);
		game->getWindow().setView(view);
	}

	StateType PlayingState::getType() const
	{
		return StateType::Playing;
	}

	void PlayingState::handleEvent(const sf::Event& event)
	{
		if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->code == sf::Keyboard::Key::Escape)
			{
				// game->getStateMachine().pushState(StateType::Settings);
			}
		}
	}

	void PlayingState::update(float dt)
	{
		if (player != nullptr)
		{
			player->update(dt, game, mapLimits);

			// --- BEZPIECZNA KAMERA (KULOODPORNA NA ROZMIAR MAPY) ---
			sf::View view = game->getWindow().getView();
			sf::Vector2f viewSize = view.getSize();

			float halfWidth = viewSize.x / 2.0f;
			float halfHeight = viewSize.y / 2.0f;

			sf::Vector2f targetCenter = player->getPosition();

			// 1. Bezpieczny Clamping dla osi X
			float minX = halfWidth;
			float maxX = mapLimits.x - halfWidth;

			if (minX > maxX)
			{
				// Ekran jest szerszy ni¿ mapa -> centrujemy na œrodku mapy
				targetCenter.x = mapLimits.x / 2.0f;
			}
			else
			{
				targetCenter.x = std::clamp(targetCenter.x, minX, maxX);
			}

			// 2. Bezpieczny Clamping dla osi Y
			float minY = halfHeight;
			float maxY = mapLimits.y - halfHeight;

			if (minY > maxY)
			{
				// Ekran jest wy¿szy ni¿ mapa -> centrujemy na œrodku mapy
				targetCenter.y = mapLimits.y / 2.0f;
			}
			else
			{
				targetCenter.y = std::clamp(targetCenter.y, minY, maxY);
			}

			view.setCenter(targetCenter);
			game->getWindow().setView(view);
		}
	}

	void PlayingState::render(sf::RenderWindow& window)
	{
		// Zawsze najpierw rysujemy t³o areny
		window.draw(*mapSprite);

		// Nastêpnie rysujemy gracza na wierzchu
		if (player != nullptr)
		{
			player->render(window);
		}
	}
}