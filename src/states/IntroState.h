// --- IntroState.h ---

#pragma once

#include "State.h"
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
    public:
        IntroState(game::Game* game);
        ~IntroState() override;

        StateType getType() const override { return StateType::Intro; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;

    private:
        void initUI();

        // --- ASYNC LOADING ---
        void loadAssetsInBg();

        // Helpers
        void loadCharactersConfig();
        void loadEnemiesConfig();
        void loadMapsConfig();
        void loadFontsAndUI();
        void loadAudio();

        // --- UI ELEMENTS ---
        sf::Texture introTexture;
        std::optional<sf::Sprite> introSprite;

        sf::RectangleShape progressBarBg;
        sf::RectangleShape progressBarFill;

        // --- THREAD & PROGRESS ---
        std::unique_ptr<std::thread> workerThread;
        std::atomic<int> loadProgress{ 0 };
        std::atomic<bool> isFinished{ false };

        float minDisplayTime{ 1.0f };
        float elapsedTime{ 0.f };
    };
}