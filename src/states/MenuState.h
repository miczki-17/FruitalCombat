// --- MenuState.h ---

#pragma once

#include "State.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <memory>

namespace game::states
{
    class MenuState : public State
    {
    private:
        // --- BACKGROUND ---
        std::vector<std::shared_ptr<sf::Texture>> bgTextures;
        std::optional<sf::Sprite> frameSprite;

        int currentFrame;
        int totalFrames;
        float frameDuration;
        float elapsedTime;

        // --- BUTTONS ---
        std::optional<sf::Sprite> startBtnSprite;
        std::optional<sf::Text> startText;

        std::optional<sf::Sprite> settingsBtnSprite;
        std::optional<sf::Sprite> shopBtnSprite;
        std::optional<sf::Sprite> achievementsBtnSprite;

        // --- EFFECTS ---
        sf::Clock clock;
        const float pulseAmplitude = 0.05f;
        const float pulseSpeed = 4.0f;

        std::string lastLangCode;
        void refreshTexts();

        // --- HELPER METHODS ---
        void initBackground();
        void initButtons();
        void updateBackgroundAnimation(float dt);
        void buttonPulse(std::optional<sf::Sprite>& btnSprite, sf::Vector2f targetSizeInPixels, std::optional<sf::Text>* linkedText = nullptr);

    public:
        MenuState(game::Game* game);
        ~MenuState() override = default;

        StateType getType() const override { return StateType::Menu; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}