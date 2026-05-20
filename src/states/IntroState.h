// --- IntroState.h --- 


#pragma once

#include "../core/State.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <thread>
#include <atomic>
#include <memory>
#include <optional>

namespace game::states
{
	class IntroState : public State
	{
	private:
		// Intro background
		sf::Texture introTexture;
		std::optional<sf::Sprite> introSprite;
		//sf::Music introMusic;

		// Progress bar elements
		sf::RectangleShape progressBarBg;
		sf::RectangleShape progressBarFill;

		// Async loading thread and flags
		std::unique_ptr<std::thread> workerThread;

		// Loading progress (0 - 100)
		std::atomic<int> loadProgress{ 0 };
		std::atomic<bool> isFinished{ false };

		// Helper variables to prevent skipping too fast
		float minDisplayTime{ 1.0f };
		float elapsedTime{ 0.f };

		// Background worker function
		void loadAssetsInBg();

	public:
		IntroState(game::Game* game);
		~IntroState();

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}