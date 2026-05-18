#pragma once

#include "../core/State.h"
#include "../utils/VideoPlayer.h"
#include <SFML/Audio.hpp>
#include <thread>
#include <atomic>
#include <memory>

namespace game::states
{
	class IntroState : public State
	{
	private:
		game::utils::VideoPlayer videoPlayer;
		sf::Music introMusic;

		float frameDuration;
		float elapsedTime;

		// W?tek i flagi ?adowania asynchronicznego
		std::unique_ptr<std::thread> workerThread;
		std::atomic<bool> isMenuLoaded{ false };
		std::atomic<bool> isConfigLoaded{ false };
		std::atomic<bool> isMapConfigLoaded{ false };
		std::atomic<bool> isUiSoundsLoaded{ false };

		void loadAssetsInBg();

	public:
		IntroState(game::Game* game);
		~IntroState();

		StateType getType() const override;

		void handleEvent(const sf::Event&) override;
		void update(float dt) override;
		void render(sf::RenderWindow&) override;
	};
}