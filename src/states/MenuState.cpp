#include "MenuState.h"
#include "../core/Game.h"
#include <format>

namespace game::states
{
	MenuState::MenuState(game::Game* game)
		: State(game), currentFrame(1), totalFrames(6), elapsedTime(0.f)
	{
		frameDuration = 1.0f / 10.0f;

		// Copy backgrounds from RAM to the GPU
		for (const auto& img : game->menuImageBuffer)
		{
			sf::Texture tex;
			if (tex.loadFromImage(img)) {
				bgTextures.push_back(std::move(tex));
			}
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

		setupButton("start", startBtnTex, startBtnSprite, { centerX, 600.0f }, { 300.0f, 120.0f });
		setupButton("settings", settingsBtnTex, settingsBtnSprite, { 0.0f, 0.0f }, { 60.0f, 60.0f });

		if (settingsBtnSprite.has_value()) {
			sf::FloatRect bounds = (*settingsBtnSprite).getGlobalBounds();
			(*settingsBtnSprite).setPosition({ viewSize.x - margin - (bounds.size.x / 2.0f), margin + (bounds.size.y / 2.0f) });
		}

		setupButton("shop", shopBtnTex, shopBtnSprite, { 0.0f, 0.0f }, { 80.0f, 80.0f });
		if (shopBtnSprite.has_value()) {
			sf::FloatRect bounds = (*shopBtnSprite).getGlobalBounds();
			(*shopBtnSprite).setPosition({ viewSize.x - margin - (bounds.size.x / 2.0f), viewSize.y - margin - (bounds.size.y / 2.0f) });
		}

		setupButton("achievements", achievementsBtnTex, achievementsBtnSprite, { 0.0f, 0.0f }, { 80.0f, 80.0f });
		if (achievementsBtnSprite.has_value()) {
			sf::FloatRect bounds = (*achievementsBtnSprite).getGlobalBounds();
			(*achievementsBtnSprite).setPosition({ viewSize.x - margin - (bounds.size.x / 2.0f) - 120.0f, viewSize.y - margin - (bounds.size.y / 2.0f) });
		}

		// Stretch the first background frame
		if (!bgTextures.empty()) {
			frameSprite = sf::Sprite(bgTextures[0]);
			sf::Vector2f introSize(bgTextures[0].getSize());
			frameSprite->setScale({ viewSize.x / introSize.x, viewSize.y / introSize.y });
			frameSprite->setPosition({ 0.f, 0.f });
		}
	
		sf::Listener::setGlobalVolume(35.0f);

		if (game->menuMusic.getStatus() != sf::SoundSource::Status::Playing)
		{
			if (game->menuMusic.openFromFile("assets/audio/menu/Victory_at_Canopy_Peak.mp3"))
			{
				game->menuMusic.setLooping(true); // Let it loop continuously
				game->menuMusic.play();
			}
			else
			{
				std::cerr << "[MENU ERROR] Cannot load bg music.\n";
			}
		}
	}

	StateType MenuState::getType() const { return StateType::Menu; }

	void MenuState::handleEvent(const sf::Event& event)
	{
		if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
		{
			if (mousePressed->button == sf::Mouse::Button::Left)
			{
				sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
				sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);

				if (startBtnSprite.has_value() && startBtnSprite->getGlobalBounds().contains(worldPos)) {
					game->playUIClick();
					game->getStateMachine().changeState(StateType::CharacterSelect);
					return;
				}
				if (settingsBtnSprite.has_value() && settingsBtnSprite->getGlobalBounds().contains(worldPos)) {
					game->playUIClick();
					game->getStateMachine().pushState(StateType::Settings);
					return;
				}
				if (achievementsBtnSprite.has_value() && achievementsBtnSprite->getGlobalBounds().contains(worldPos)) {
					game->playUIClick();
				}
				if (shopBtnSprite.has_value() && shopBtnSprite->getGlobalBounds().contains(worldPos)) {
					game->playUIClick();
				}
			}
		}
	}

	void MenuState::update(float dt)
	{
		if (bgTextures.empty() || !frameSprite.has_value()) return;

		elapsedTime += dt;
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		static bool isReverse = false;

		// Ping-pong background animation
		while (elapsedTime >= frameDuration) {
			elapsedTime -= frameDuration;
			if (!isReverse) currentFrame++; else currentFrame--;
			if (currentFrame >= bgTextures.size()) { currentFrame = bgTextures.size(); isReverse = true; }
			else if (currentFrame <= 1) { currentFrame = 1; isReverse = false; }

			const sf::Texture& nextTex = bgTextures[currentFrame - 1];
			frameSprite->setTexture(nextTex, true);
			sf::Vector2f introSize(nextTex.getSize());
			frameSprite->setScale({ viewSize.x / introSize.x, viewSize.y / introSize.y });
		}

		sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
		sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);

		// Helper for updating button hover states
		auto updateHover = [&](std::optional<sf::Sprite>& btn, sf::Vector2f targetSize) {
			if (!btn) return;

			// Calculate the base scale to know our starting size
			sf::Vector2f texSize(btn->getTexture().getSize());
			float baseScaleX = targetSize.x / texSize.x;
			float baseScaleY = targetSize.y / texSize.y;

			// 3. Check if the mouse is hovering over the button
			if (btn->getGlobalBounds().contains(worldPos)) {
				btn->setColor(sf::Color(255, 255, 255));
				// HERE IS THE ENLARGEMENT MAGIC:
				btn->setScale({ baseScaleX * 1.1f, baseScaleY * 1.1f });
			}
			else {
				btn->setColor(sf::Color(210, 210, 210)); // Slightly dimmed
				btn->setScale({ baseScaleX, baseScaleY }); // Return to normal size
			}
		};

		updateHover(startBtnSprite, { 320.0f, 140.0f });
		updateHover(settingsBtnSprite, { 70.0f, 70.0f });
		updateHover(shopBtnSprite, { 90.0f, 90.0f });
		updateHover(achievementsBtnSprite, { 90.0f, 90.0f });

		// Pulsing effect for the main start button
		if (startBtnSprite.has_value()) buttonPulse(startBtnSprite, { 300.0f, 120.0f });
	}

	void MenuState::render(sf::RenderWindow& window)
	{
		if (frameSprite.has_value()) window.draw(*frameSprite);
		if (startBtnSprite.has_value()) window.draw(*startBtnSprite);
		if (settingsBtnSprite.has_value()) window.draw(*settingsBtnSprite);
		if (shopBtnSprite.has_value()) window.draw(*shopBtnSprite);
		if (achievementsBtnSprite.has_value()) window.draw(*achievementsBtnSprite);

		game->drawMenuCursor();
	}

	void MenuState::buttonPulse(std::optional<sf::Sprite>& btnSprite, sf::Vector2f targetSizeInPixels)
	{
		float time = clock.getElapsedTime().asSeconds();
		float pulse = 1.0f + pulseAmplitude * std::sin(time * pulseSpeed); // Using defined constants
		sf::Vector2f originalSize(btnSprite->getTexture().getSize());
		btnSprite->setScale({ (targetSizeInPixels.x / originalSize.x) * pulse, (targetSizeInPixels.y / originalSize.y) * pulse });
	}
}