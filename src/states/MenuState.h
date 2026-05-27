// --- MenuState.h ---

#pragma once

#include "State.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <optional>
#include <vector>
#include <memory>

namespace game::states
{
    class MenuState : public State
    {
    private:
        std::vector<std::shared_ptr<sf::Texture>> bgTextures;
        std::optional<sf::Sprite> frameSprite;


        std::optional<sf::Sprite> startBtnSprite;
        std::optional<sf::Text> startText;

        std::optional<sf::Sprite> settingsBtnSprite;
        std::optional<sf::Sprite> shopBtnSprite;
        std::optional<sf::Sprite> achievementsBtnSprite;
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