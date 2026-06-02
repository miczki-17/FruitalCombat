// --- MapSelectState.h ---

#pragma once

#include "State.h"
#include <SFML/Graphics.hpp>
#include <deque>
#include <vector>
#include <optional>
#include <string>

namespace game::states
{
    class MapSelectState : public State
    {
    private:
        std::optional<sf::Sprite> bgSprite;
        sf::RectangleShape darkOverlay;

        std::optional<sf::Sprite> leftArrowSprite, rightArrowSprite;
        std::optional<sf::Sprite> selectBtnSprite, backBtnSprite;
        std::optional<sf::Text> selectBtnText;

        std::optional<sf::Text> mapNameText;
        std::optional<sf::Text> mapDescText;
        std::optional<sf::Text> mapStatsText;

        std::optional<sf::Sprite> starFullSprite, starEmptySprite;

        sf::Clock animationClock;

        struct Firefly {
            sf::Vector2f position;
            float speed = 0.0f, alpha = 0.0f, lifetime = 0.0f, maxLifetime = 0.0f, size = 0.0f, swayOffset = 0.0f;
        };
        std::vector<Firefly> fireflies;

        struct MapOption {
            std::string jsonKey;
            std::string name;
            std::string description;
            std::optional<sf::Sprite> thumbnailSprite;
            int difficultyStars = 1;
            float damageMultiplier = 1.0f;
        };

        std::deque<MapOption> roster;

        int targetIndex;
        float currentScroll;

        void initUI();
        void initFireflies();
        void updateFireflies(float dt);
        void loadRoster();

    public:
        MapSelectState(game::Game* game);
        ~MapSelectState() override = default;

        StateType getType() const override { return StateType::MapSelect; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}