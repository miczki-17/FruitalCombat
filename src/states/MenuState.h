#pragma once

#include "../core/State.h"


namespace game::states
{
	class MenuState : public State
	{
	private:
		// bg textures vector
		std::vector<sf::Texture> bgTextures;
		
		std::optional<sf::Sprite> frameSprite;
		sf::Music bgMusic;


		// menu Btns
		sf::Texture startBtnTex;
		std::optional<sf::Sprite>  startBtnSprite;

		sf::Texture settingsBtnTex;
		std::optional<sf::Sprite>  settingsBtnSprite;

		sf::Texture shopBtnTex;
		std::optional<sf::Sprite>  shopBtnSprite;

		sf::Texture achievementsBtnTex;
		std::optional<sf::Sprite>  achievementsBtnSprite;

		sf::Texture backBtnTex;
		std::optional<sf::Sprite>  backBtnSprite;



		// bg logic
		int currentFrame;
		int totalFrames;
		float frameDuration;
		float elapsedTime;


		// buttons logic
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


		void buttonPulse(std::optional<sf::Sprite>& btnSprite, sf::Vector2f targetSizeInPixels);
	};
}