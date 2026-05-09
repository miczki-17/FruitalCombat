#include "MenuState.h"
#include "../core/Game.h"
#include <format>

namespace game::states
{
	MenuState::MenuState(game::Game* game)
		: State(game), currentFrame(1), totalFrames(6), elapsedTime(0.f)
	{
		frameDuration = 1.0f / 10.0f;

		// copy loaded images
		for (const auto& img : game->menuImageBuffer)
		{
			sf::Texture tex;
			if (tex.loadFromImage(img))
			{
				bgTextures.push_back(std::move(tex));
			}
		}

		// clear buffer
		//game->menuImageBuffer.clear();


		auto setupButton = [&](const std::string& key, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
		{
			if (game->menuUiBuffer.contains(key))
			{
				// check this key
				if (tex.loadFromImage(game->menuUiBuffer[key]))
				{
					spr = sf::Sprite(tex);

					// download original size
					sf::Vector2f originalSize(tex.getSize());

					// calculate scale
					float scaleX = targetSize.x / originalSize.x;
					float scaleY = targetSize.y / originalSize.y;
					(*spr).setScale({ scaleX, scaleY });

					// set origin
					(*spr).setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });

					// set pos
					(*spr).setPosition(pos);
				}
			}
			else
			{
				std::cerr << "[MENU ERROR] can not find " << key << " in buffer\n";
			}
		};


		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float centerX = viewSize.x / 2.0f;
		float margin = 20.0f;

		// start button
		setupButton("start", startBtnTex, startBtnSprite, { centerX, 600.0f }, { 300.0f, 120.0f });

		// settings button
		setupButton("settings", settingsBtnTex, settingsBtnSprite, { 0.0f, 0.0f }, { 60.0f, 60.0f });
		if (settingsBtnSprite.has_value())
		{
			sf::FloatRect bounds = (*settingsBtnSprite).getGlobalBounds();

			float topRightX = viewSize.x - margin - (bounds.size.x / 2.0f);
			float topRightY = margin + (bounds.size.y / 2.0f);
			(*settingsBtnSprite).setPosition({ topRightX, topRightY });
		}

		// shop button
		setupButton("shop", shopBtnTex, shopBtnSprite, { 0.0f, 0.0f }, { 100.0f, 100.0f });
		if (shopBtnSprite.has_value())
		{
			sf::FloatRect bounds = (*shopBtnSprite).getGlobalBounds();

			float downRightX = viewSize.x - margin - (bounds.size.x / 2.0f);
			float downRightY = viewSize.y - margin - (bounds.size.y / 2.0f);
			(*shopBtnSprite).setPosition({ downRightX, downRightY });
		}

		// achievements button
		setupButton("achievements", achievementsBtnTex, achievementsBtnSprite, { 0.0f, 0.0f }, { 100.0f, 100.0f });
		if (achievementsBtnSprite.has_value())
		{
			sf::FloatRect bounds = (*achievementsBtnSprite).getGlobalBounds();

			float downRightX = viewSize.x - margin - (bounds.size.x / 2.0f);
			float downRightY = viewSize.y - margin - (bounds.size.y / 2.0f);
			(*achievementsBtnSprite).setPosition({ downRightX - 120.0f, downRightY });
		}

		// clear buffer
		//game->menuUiBuffer.clear();



		// strech + scale
		if (!bgTextures.empty())
		{
			frameSprite = sf::Sprite(bgTextures[0]);

			sf::Vector2f viewSize = game->getWindow().getView().getSize();
			sf::Vector2f introSize(bgTextures[0].getSize());

			float scaleX = static_cast<float>(viewSize.x) / introSize.x;
			float scaleY = static_cast<float>(viewSize.y) / introSize.y;

			(*frameSprite).setScale({ scaleX, scaleY });

			(*frameSprite).setPosition({ 0.f, 0.f });
			(*frameSprite).setOrigin({ 0.f, 0.f });
		}

		// music start
		if (bgMusic.openFromFile("../../../assets/audio/menu/Victory_at_Canopy_Peak.mp3"))
		{
			//bgMusic.setLooping(true);
			bgMusic.play();
		}
		else
		{
			std::cerr << "[MENU ERROR] can not load bg music.\n";
		}
	}


	StateType MenuState::getType() const
	{
		return StateType::Menu;
	}

	// HANDLE EVENTS
	void MenuState::handleEvent(const sf::Event& event)
	{
		if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
		{
			if (mousePressed->button == sf::Mouse::Button::Left)
			{
				sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
				sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);
				if (startBtnSprite->getGlobalBounds().contains(worldPos))
				{
					// PLAYING state
					game->getStateMachine().changeState(StateType::Playing);
				}
			}
		}
	}

	// UPDATE
	void MenuState::update(float dt)
	{
		elapsedTime += dt;
		sf::Vector2f viewSize = game->getWindow().getView().getSize();

		static bool isReverse = false;

		while (elapsedTime >= frameDuration)
		{
			elapsedTime -= frameDuration;

			if (!isReverse)
				currentFrame++;
			else
				currentFrame--;

			if (currentFrame >= bgTextures.size())
			{
				currentFrame = bgTextures.size();
				isReverse = true;
			}
			else if (currentFrame <= 1)
			{
				currentFrame = 1;
				isReverse = false;
			}

			const sf::Texture& nextTex = bgTextures[currentFrame - 1];
			frameSprite->setTexture(nextTex, true);

			sf::Vector2f introSize(nextTex.getSize());
			float scaleX = viewSize.x / introSize.x;
			float scaleY = viewSize.y / introSize.y;

			frameSprite->setScale({ scaleX, scaleY });
		}
	}

	// RENDER
	void MenuState::render(sf::RenderWindow& window)
	{
		// draw bg
		if (frameSprite.has_value())		window.draw(*frameSprite);

		// draw Btns
		if (startBtnSprite.has_value())			window.draw(*startBtnSprite);
		if (settingsBtnSprite.has_value())		window.draw(*settingsBtnSprite);
		if (shopBtnSprite.has_value())			window.draw(*shopBtnSprite);
		if (achievementsBtnSprite.has_value())	window.draw(*achievementsBtnSprite);


		// UI animate
		buttonPulse(startBtnSprite, {300.0f, 120.0f});
	}



	void MenuState::buttonPulse(std::optional<sf::Sprite>& btnSprite, sf::Vector2f targetSizeInPixels)
	{
		float time = clock.getElapsedTime().asSeconds();
		float pulse = 1.0f + 0.05f * std::sin(time * 4.0f);

		// take original size
		sf::Vector2f originalSize(btnSprite->getTexture().getSize());

		// calculate scale
		float scaleX = targetSizeInPixels.x / originalSize.x;
		float scaleY = targetSizeInPixels.y / originalSize.y;

		// pulse
		btnSprite->setScale({ scaleX * pulse, scaleY * pulse });
	}
}
