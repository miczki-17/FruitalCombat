#pragma once

#include "../core/State.h"
#include <thread>
#include <atomic>


namespace game::states
{
	class IntroState : public State
	{
	private:
		// intro textures vector
		std::vector<sf::Texture> introTextures;

		std::optional<sf::Sprite> frameSprite;
		sf::Music introMusic;

		// intro logic
		int currentFrame;
		int totalFrames;
		float frameDuration;
		float elapsedTime;


		// async menu loading
		std::unique_ptr<std::thread> workerThread;
		std::atomic<bool> isMenuLoaded{ false };

		void loadMenuAssetsInBg();


		//void loadNextFrame();

	public:
		IntroState(game::Game* game);
		~IntroState();

		StateType getType() const override;

		void handleEvent(const sf::Event&) override;
		void update(float dt) override;
		void render(sf::RenderWindow&) override;
	};
}