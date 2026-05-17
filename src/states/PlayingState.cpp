#include "PlayingState.h"
#include "../factories/FruitFactory.h"
#include "../core/ArenaContext.h"
#include <algorithm>
#include <iostream>

namespace game::states
{
	PlayingState::PlayingState(game::Game* game)
		: State(game)
	{
		std::cout << "[PLAYING STATE] Budowanie areny d¿ungli...\n";

		std::string mapKey = game->selectedMapKey;
		const auto& mapData = game->mapsConfig[mapKey];
		std::string mapPath = mapData.value("texturePath", "");
		std::string mapMaskPath = mapData.value("maskPath", "");

		// 1. £adowanie mapy wizualnej
		if (mapTexture.loadFromFile(mapPath))
		{
			mapSprite.emplace(mapTexture);
			mapSprite->setScale({ mapScale, mapScale });

			sf::Vector2u rawSize = mapTexture.getSize();
			mapLimits = sf::Vector2f(rawSize.x * mapScale, rawSize.y * mapScale);
		}

		// 2. £adowanie maski kolizji do RAM
		if (!collisionMask.loadFromFile(mapMaskPath))
		{
			std::cerr << "[B£¥D] Nie mo¿na za³adowaæ pliku " << mapMaskPath << '\n';
		}

		// 3. £adowanie zasobów interfejsu (HUD)
		if (!uiFont.openFromFile("../../../assets/fonts/Arial.TTF"))
		{
			std::cerr << "[OSTRZE¯ENIE] Brak czcionki interfejsu!\n";
		}

		/*if (coinIconTexture.loadFromFile("../../../assets/textures/ui/coin_icon.png"))
		{
			coinIconSprite.emplace(coinIconTexture);
		}*/


		game::ArenaContext context{ bullets };
		game::factories::FruitFactory factory(context, game->fruitsConfig);
		player = factory.createFruit(game->selectedFruitType);

		if (player != nullptr)
		{
			player->setPosition({ mapLimits.x / 2.0f, mapLimits.y / 2.0f });
		}

		// Inicjalizacja kamery
		cameraView = game->getWindow().getDefaultView();
		cameraView.zoom(1.4f); // Oddalenie o 40%
	}

	StateType PlayingState::getType() const
	{
		return StateType::Playing;
	}

	void PlayingState::handleEvent(const sf::Event& event)
	{
		// Zoom rolk? myszy
		if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>())
		{
			if (scroll->delta > 0)      cameraView.zoom(0.9f);
			else if (scroll->delta < 0) cameraView.zoom(1.1f);
		}

		// Obs?uga klawiszy (Pauza oraz DASH)
		if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
		{
			// Pauza
			if (keyPressed->code == sf::Keyboard::Key::Escape)
			{
				game->getStateMachine().pushState(StateType::Settings);
			}

			// --- JEDNORAZOWY DASH POD SPACJ? ---
			if (keyPressed->code == sf::Keyboard::Key::LShift)
			{
				if (player != nullptr)
				{
					sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
					sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, cameraView);

					player->useSkill(mouseWorldPos);
				}
			}
		}
	}

	void PlayingState::update(float dt)
	{
		if (player != nullptr)
		{
			// 1. Aktualizacja ruchu i fizyki gracza
			player->update(dt, game, mapLimits, collisionMask, mapScale);

			// --- 2. OBS?UGA ATAKU CI?G?EGO (Strzelba) ---
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			{
				sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
				sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, cameraView);

				player->useWeapon(mouseWorldPos);
			}

			// --- 3. AKTUALIZACJA I CZYSZCZENIE POCISKÓW ---
			for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; --i)
			{
				bullets[i].update(dt, collisionMask, mapScale);

				if (!bullets[i].getIsActive())
				{
					bullets.erase(bullets.begin() + i);
				}
			}

			// --- 4. BEZPIECZNA KAMERA (CLAMPING) ---
			sf::Vector2f viewSize = cameraView.getSize();
			float halfWidth = viewSize.x / 2.0f;
			float halfHeight = viewSize.y / 2.0f;

			sf::Vector2f targetCenter = player->getPosition();

			float minX = halfWidth;
			float maxX = mapLimits.x - halfWidth;
			float minY = halfHeight;
			float maxY = mapLimits.y - halfHeight;

			if (maxX < minX) targetCenter.x = mapLimits.x / 2.0f;
			else             targetCenter.x = std::clamp(targetCenter.x, minX, maxX);

			if (maxY < minY) targetCenter.y = mapLimits.y / 2.0f;
			else             targetCenter.y = std::clamp(targetCenter.y, minY, maxY);

			cameraView.setCenter(targetCenter);
			game->getWindow().setView(cameraView);
		}
	}

	void PlayingState::renderHUD(sf::RenderWindow& window)
	{
		/*if (player == nullptr) return;

		window.setView(window.getDefaultView());

		float hpBgWidth = 200.0f;
		float hpBgHeight = 28.0f;
		sf::Vector2f startPos(20.0f, 20.0f);

		sf::RectangleShape hpBgFrame({ hpBgWidth, hpBgHeight });
		hpBgFrame.setPosition(startPos);
		hpBgFrame.setFillColor(sf::Color(150, 0, 0));
		hpBgFrame.setOutlineThickness(4.0f);
		hpBgFrame.setOutlineColor(sf::Color::Black);
		window.draw(hpBgFrame);

		float hpRatio = static_cast<float>(player->getHp()) / player->getMaxHp();
		sf::RectangleShape hpFill({ hpBgWidth * hpRatio, hpBgHeight });
		hpFill.setPosition(startPos);
		hpFill.setFillColor(sf::Color(220, 0, 0));
		window.draw(hpFill);

		sf::Text hpText(uiFont);
		hpText.setString(std::to_string(player->getHp()) + " / " + std::to_string(player->getMaxHp()));
		hpText.setCharacterSize(16);
		hpText.setFillColor(sf::Color::White);
		sf::FloatRect hpBounds = hpText.getLocalBounds();
		hpText.setPosition({ startPos.x + (hpBgWidth - hpBounds.size.x) / 2.0f, startPos.y + 3.0f });
		window.draw(hpText);

		sf::Vector2f xpPos(startPos.x, startPos.y + hpBgHeight + 12.0f);
		sf::RectangleShape xpBgFrame({ hpBgWidth, 22.0f });
		xpBgFrame.setPosition(xpPos);
		xpBgFrame.setFillColor(sf::Color(50, 50, 50));
		xpBgFrame.setOutlineThickness(4.0f);
		xpBgFrame.setOutlineColor(sf::Color::Black);
		window.draw(xpBgFrame);

		sf::RectangleShape xpFill({ hpBgWidth * 0.25f, 22.0f });
		xpFill.setPosition(xpPos);
		xpFill.setFillColor(sf::Color(50, 220, 50));
		window.draw(xpFill);

		sf::Text lvlText(uiFont);
		lvlText.setString("LV " + std::to_string(player->getLevel()));
		lvlText.setCharacterSize(14);
		lvlText.setFillColor(sf::Color::White);
		sf::FloatRect lvlBounds = lvlText.getLocalBounds();
		lvlText.setPosition({ xpPos.x + hpBgWidth - lvlBounds.size.x - 8.0f, xpPos.y + 2.0f });
		window.draw(lvlText);

		sf::Vector2f coinPos(startPos.x, xpPos.y + 34.0f);
		if (coinIconSprite.has_value())
		{
			coinIconSprite->setPosition(coinPos);
			coinIconSprite->setScale({ 1.5f, 1.5f });
			window.draw(*coinIconSprite);
		}

		sf::Text coinText(uiFont);
		coinText.setString(std::to_string(player->getCoins()));
		coinText.setCharacterSize(24);
		coinText.setFillColor(sf::Color::White);
		coinText.setOutlineThickness(2.0f);
		coinText.setOutlineColor(sf::Color::Black);
		coinText.setPosition({ coinPos.x + 36.0f, coinPos.y - 2.0f });
		window.draw(coinText);*/
	}

	void PlayingState::render(sf::RenderWindow& window)
	{
		window.setView(cameraView);

		if (mapSprite.has_value()) window.draw(*mapSprite);

		for (auto& bullet : bullets)
		{
			bullet.render(window);
		}

		if (player != nullptr)
		{
			player->render(window);
		}

		renderHUD(window);
	}
}