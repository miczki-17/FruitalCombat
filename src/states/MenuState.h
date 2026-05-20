// --- MenuState.h --- 


#pragma once

#include "../core/State.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <optional>
#include <vector>

namespace game::states
{
	class MenuState : public State
	{
	private:
		// Background textures vector
		std::vector<sf::Texture> bgTextures;

		std::optional<sf::Sprite> frameSprite;

		// Menu Buttons
		sf::Texture startBtnTex;
		std::optional<sf::Sprite> startBtnSprite;

		// START text
		sf::Font customFont;
		std::optional<sf::Text> startText;

		sf::Texture settingsBtnTex;
		std::optional<sf::Sprite> settingsBtnSprite;

		sf::Texture shopBtnTex;
		std::optional<sf::Sprite> shopBtnSprite;

		sf::Texture achievementsBtnTex;
		std::optional<sf::Sprite> achievementsBtnSprite;

		sf::Texture backBtnTex;
		std::optional<sf::Sprite> backBtnSprite;

		// Background animation logic
		int currentFrame;
		int totalFrames;
		float frameDuration;
		float elapsedTime;

		// Button logic properties
		const float baseScale = 1.0f;
		const float pulseAmplitude = 0.05f;
		const float pulseSpeed = 4.0f;
		sf::Clock clock;

	public:
		MenuState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;

		void buttonPulse(std::optional<sf::Sprite>& btnSprite, sf::Vector2f targetSizeInPixels, std::optional<sf::Text>* linkedText = nullptr);
	};
}