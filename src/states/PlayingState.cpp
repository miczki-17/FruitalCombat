#include "PlayingState.h"
#include "../factories/FruitFactory.h"
#include "../core/ArenaContext.h"
#include <algorithm>
#include <iostream>
#include <random> 

namespace game::states
{
	// Sludge puddle instance blueprint
	struct AcidPuddle {
		sf::CircleShape shape;
		float lifetime = 4.0f;
		float radius = 55.0f;
	};

	// Container allocating current map puddle layers
	static std::vector<AcidPuddle> acidPuddles;

	PlayingState::PlayingState(game::Game* game)
		: State(game)
	{
		std::cout << "[PLAYING STATE] arena building...\n";

		std::string mapKey = game->selectedMapKey;
		const auto& mapData = game->mapsConfig[mapKey];
		std::string mapPath = mapData.value("texturePath", "");
		std::string mapMaskPath = mapData.value("maskPath", "");

		// 1. Map texture layout loads
		if (mapTexture.loadFromFile(mapPath))
		{
			mapSprite.emplace(mapTexture);
			mapSprite->setScale({ mapScale, mapScale });

			sf::Vector2u rawSize = mapTexture.getSize();
			mapLimits = sf::Vector2f(rawSize.x * mapScale, rawSize.y * mapScale);
		}

		// 2. Collision mask array buffer cache
		if (!collisionMask.loadFromFile(mapMaskPath))
		{
			std::cerr << "[ERROR] can not load file " << mapMaskPath << '\n';
		}

		// 3. User interface HUD resources allocation
		if (!uiFont.openFromFile("../../../assets/fonts/Arial.TTF"))
		{
			std::cerr << "[WARNING] lack of font!\n";
		}

		if (game->menuUiBuffer.contains("crosshair"))
		{
			if (crosshairTex.loadFromImage(game->menuUiBuffer["crosshair"]))
			{
				crosshairSprite.emplace(crosshairTex);
				sf::Vector2u size = crosshairTex.getSize();

				crosshairSprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
				game->getWindow().setMouseCursorVisible(false);
			}
		}
		else
		{
			std::cerr << "[WARNING] lack of crosshair!\n";
		}

		// Helper to configure and load button graphics
		auto setupButton = [&](const std::string& key, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
			{
				if (game->menuUiBuffer.contains(key))
				{
					if (tex.loadFromImage(game->menuUiBuffer[key]))
					{
						spr = sf::Sprite(tex);
						sf::Vector2f originalSize(tex.getSize());
						float scaleX = targetSize.x / originalSize.x;
						float scaleY = targetSize.y / originalSize.y;
						(*spr).setScale({ scaleX, scaleY });
						(*spr).setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });
						(*spr).setPosition(pos);
					}
				}
				else {
					std::cerr << "[MENU ERROR] cannot find " << key << " in buffer\n";
				}
			};
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float centerX = viewSize.x / 2.0f;
		float margin = 20.0f;

		setupButton("settings", settingsBtnTex, settingsBtnSprite, { 0.0f, 0.0f }, { 60.0f, 60.0f });

		if (settingsBtnSprite.has_value()) {
			sf::FloatRect bounds = (*settingsBtnSprite).getGlobalBounds();
			(*settingsBtnSprite).setPosition({ viewSize.x - margin - (bounds.size.x / 2.0f), margin + (bounds.size.y / 2.0f) });
		}

		game::ArenaContext context{ bullets };
		game::factories::FruitFactory factory(context, game->fruitsConfig);
		player = factory.createFruit(game->selectedFruitType);

		if (player != nullptr)
		{
			player->setPosition({ mapLimits.x / 2.0f, mapLimits.y / 2.0f });
		}

		cameraView = game->getWindow().getDefaultView();
		cameraView.zoom(1.4f);
	}

	StateType PlayingState::getType() const
	{
		return StateType::Playing;
	}

	void PlayingState::handleEvent(const sf::Event& event)
	{
		if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>())
		{
			if (scroll->delta > 0)      cameraView.zoom(0.9f);
			else if (scroll->delta < 0) cameraView.zoom(1.1f);
		}

		if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->code == sf::Keyboard::Key::Escape)
			{
				game->getStateMachine().pushState(StateType::Pause);
			}

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

		if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
		{
			if (mousePressed->button == sf::Mouse::Button::Left)
			{
				sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
				sf::Vector2f uiPos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());
				if (settingsBtnSprite.has_value() && settingsBtnSprite->getGlobalBounds().contains(uiPos))
				{
					game->playUIClick();
					game->getStateMachine().pushState(StateType::Pause);
					return;
				}
			}
		}
	}

	void PlayingState::update(float dt)
	{
		static float shakeIntensity = 0.0f;

		// --- IMPACT SIMULATION CHEAT FOR DEV TESTING --- 
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H))
		{
			shakeIntensity = 20.0f;
			sf::sleep(sf::milliseconds(45)); // Hit-Stop game feel freezes
		}

		if (player != nullptr)
		{
			// 1. Player moves & physics
			player->update(dt, game, mapLimits, collisionMask, mapScale);

			// 2. Attack execution
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			{
				sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
				sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, cameraView);

				player->useWeapon(mouseWorldPos);
			}

			// 3. Update bullets & handle splash impact drops
			for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; --i)
			{
				sf::Vector2f explodePos = bullets[i].getPosition();
				bullets[i].update(dt, collisionMask, mapScale);

				if (!bullets[i].getIsActive())
				{
					// If mortar shell drops (detected using radius footprint)
					if (bullets[i].getRadius() > 10.0f)
					{
						AcidPuddle puddle;
						puddle.shape.setRadius(puddle.radius);
						puddle.shape.setOrigin({ puddle.radius, puddle.radius });
						puddle.shape.setPosition(explodePos);
						puddle.shape.setFillColor(sf::Color(255, 100, 0, 130)); // Translucent sludge

						acidPuddles.push_back(puddle);
						shakeIntensity = 8.0f; // Soft juice screenshake on splashdown
					}
					bullets.erase(bullets.begin() + i);
				}
			}

			// 3.5 Fade puddle entities
			for (int i = static_cast<int>(acidPuddles.size()) - 1; i >= 0; --i) {
				acidPuddles[i].lifetime -= dt;

				if (acidPuddles[i].lifetime < 1.0f) {
					sf::Color c = acidPuddles[i].shape.getFillColor();
					c.a = static_cast<uint8_t>(130 * acidPuddles[i].lifetime);
					acidPuddles[i].shape.setFillColor(c);
				}

				if (acidPuddles[i].lifetime <= 0.0f) {
					acidPuddles.erase(acidPuddles.begin() + i);
				}
			}

			// 4. Save camera adjustments (CLAMPING)
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

			// 4.5 Shake vector implementation logic
			if (shakeIntensity > 0.0f)
			{
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<float> offset(-shakeIntensity, shakeIntensity);

				cameraView.move({ offset(gen), offset(gen) });

				shakeIntensity -= dt * 50.0f;
				if (shakeIntensity < 0.0f) shakeIntensity = 0.0f;
			}

			game->getWindow().setView(cameraView);

			// 5. Interface button hover interactions
			sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
			sf::Vector2f uiHoverPos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());

			auto updateHover = [&](std::optional<sf::Sprite>& btn, sf::Vector2f targetSize) {
				if (!btn) return;

				sf::Vector2f texSize(btn->getTexture().getSize());
				float baseScaleX = targetSize.x / texSize.x;
				float baseScaleY = targetSize.y / texSize.y;

				if (btn->getGlobalBounds().contains(uiHoverPos)) {
					btn->setColor(sf::Color(255, 255, 255));
					btn->setScale({ baseScaleX * 1.1f, baseScaleY * 1.1f });
				}
				else {
					btn->setColor(sf::Color(210, 210, 210));
					btn->setScale({ baseScaleX, baseScaleY });
				}
				};
			updateHover(settingsBtnSprite, { 60.0f, 60.0f });
		}
	}

	void PlayingState::renderHUD(sf::RenderWindow& window) {}

	void PlayingState::render(sf::RenderWindow& window)
	{
		window.setView(cameraView);

		if (mapSprite.has_value()) window.draw(*mapSprite);

		// Ground layer rendering context for acid puddles
		for (auto& puddle : acidPuddles) {
			window.draw(puddle.shape);
		}

		for (auto& bullet : bullets)
		{
			bullet.render(window);
		}

		if (player != nullptr)
		{
			player->render(window);
		}

		window.setView(window.getDefaultView());

		if (settingsBtnSprite.has_value()) window.draw(*settingsBtnSprite);

		renderHUD(window);

		if (game->getStateMachine().getCurrentStateType() == states::StateType::Playing)
		{
			if (crosshairSprite.has_value())
			{
				sf::View oldView = window.getView();
				window.setView(window.getDefaultView());
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				crosshairSprite->setPosition({ static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) });
				window.draw(*crosshairSprite);
				window.setView(oldView);
			}
		}
	}
}