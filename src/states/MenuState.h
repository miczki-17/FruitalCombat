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

		// bg logic
		int currentFrame;
		int totalFrames;
		float frameDuration;
		float elapsedTime;

	public:
		MenuState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}