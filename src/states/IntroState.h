#pragma once

#include <iostream>
#include <optional>

#include <SFML/Audio.hpp>
#include "../core/State.h"

namespace game::states
{
	class IntroState : public State
	{
	private:
		std::vector<sf::Texture> introTextures;

		sf::Texture frameTexture;
		std::optional<sf::Sprite> frameSprite;
		sf::Music introMusic;

		int currentFrame;
		int totalFrames;
		float frameDuration;
		float elapsedTime;

		void loadNextFrame();

	public:
		IntroState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event&) override;
		void update(float dt) override;
		void render(sf::RenderWindow&) override;
	};
}