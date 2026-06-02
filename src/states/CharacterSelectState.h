// --- characterSelectState.h ---

#pragma once

#include "State.h"
#include "../entities/EntityTypes.h"
#include <SFML/Graphics.hpp>
#include <deque>
#include <vector>
#include <optional>
#include <string>
#include <cstdint>

namespace game::states
{
    class CharacterSelectState : public State
    {
    private:
        // --- BACKGROUND AND OVERLAY ---
        std::optional<sf::Sprite> bgSprite;
        sf::RectangleShape darkOverlay;

        // --- UI BUTTONS ---
        std::optional<sf::Sprite> leftArrowSprite, rightArrowSprite;
        std::optional<sf::Sprite> selectBtnSprite, backBtnSprite;
        std::optional<sf::Text> selectBtnText;

        // --- UI ICONS & STAT BARS ---
        std::optional<sf::Sprite> hpIconSprite, dmgIconSprite, spdIconSprite;
        std::optional<sf::Sprite> statBarFrameSprite;
        std::optional<sf::Sprite> statBarFillSprite;

        // --- TYPOGRAPHY ---
        std::optional<sf::Text> characterNameText;
        std::optional<sf::Text> characterTitleText;
        std::optional<sf::Text> abilitiesTextDisplay;

        sf::Clock animationClock;

        // --- MAGICAL FIREFLIES ---
        struct Firefly {
            sf::Vector2f position;
            float speed = 0.0f, alpha = 0.0f, lifetime = 0.0f, maxLifetime = 0.0f, size = 0.0f, swayOffset = 0.0f;
        };
        std::vector<Firefly> fireflies;

        // --- FRUIT DATA ---
        struct FruitOption {
            game::entities::FruitType type = game::entities::FruitType::Apple;
            std::string jsonKey, displayName, title, abilitiesText;

            std::optional<sf::Sprite> sprite;
            std::optional<sf::Sprite> platformSprite;

            // Start Animation
            std::vector<sf::IntRect> startAnimationFrames;
            bool hasStartAnimation = false, isPlayingStartAnimation = false, hasPlayedStartAnimation = false;

            // Idle Animation
            bool isAnimated = false;
            std::vector<sf::IntRect> animationFrames;
            int currentFrameIndex = 0;
            float animationTimer = 0.0f;

            int hp = 0, damage = 0, speed = 0;
        };

        std::deque<FruitOption> roster;
        int targetIndex;
        float currentScroll;
        std::vector<std::pair<float, int>> renderZOrder;

        // --- HELPER METHODS ---
        void initUI();
        void initFireflies();
        void updateFireflies(float dt);
        void loadRoster();
        void drawStatBar(sf::RenderWindow& window, std::optional<sf::Sprite>& icon, int value, float gameMaxValue, sf::Vector2f pos, sf::Color barColor, const std::string& labelText);

    public:
        CharacterSelectState(game::Game* game);
        StateType getType() const override { return StateType::CharacterSelect; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}