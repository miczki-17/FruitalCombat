// --- DeathState.h ---

#pragma once

#include "State.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

namespace game::states
{
    class DeathState : public State
    {
    public:
        DeathState(game::Game* game);
        ~DeathState() override = default;

        StateType getType() const override { return StateType::Death; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;

    private:
        void initUI();
        void initLabels(float centerX, float viewHeight);
        void initButtons(float centerX, float viewHeight);

        sf::RectangleShape backgroundOverlay_;

        std::optional<sf::Text> youDiedText_;
        std::optional<sf::Text> killedByLabel_;
        std::optional<sf::Text> killerNameText_;

        // Statystyki
        std::optional<sf::Sprite> biomassIcon_;
        std::optional<sf::Text> scoreText_;
        std::optional<sf::Text> wavesText_;

        // Przyciski i ich etykiety
        std::optional<sf::Sprite> retryBtnSprite_;
        std::optional<sf::Text> retryBtnText_;

        std::optional<sf::Sprite> menuBtnSprite_;
        std::optional<sf::Text> menuBtnText_;
    };
}