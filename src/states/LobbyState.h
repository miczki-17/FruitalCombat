// --- LobbyState.h ---

#pragma once

#include "State.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

namespace game::states
{
    class LobbyState : public State
    {
    private:
        std::optional<sf::Sprite> bgSprite;
        std::optional<sf::Sprite> characterSprite;
        std::optional<sf::Sprite> characterPlatformSprite;

        // --- BUTTONS ---
        std::optional<sf::Sprite> settingsBtnSprite;
        std::optional<sf::Sprite> shopBtnSprite;
        std::optional<sf::Sprite> achievementsBtnSprite;

        std::optional<sf::Sprite> playBtnSprite;
        std::optional<sf::Text> playBtnText;

        std::optional<sf::Sprite> mapSelectBtnSprite;
        std::optional<sf::Text> mapSelectBtnText;

        std::string lastLangCode;

        // --- ANIMATION & JUICE ---
        std::vector<sf::IntRect> animFrames;
        int currentFrameIndex = 0;
        float animationTimer = 0.0f;
        bool isAnimated = false;

        sf::Clock animationClock;
        std::optional<sf::Text> charNameText;

        // COINS
        std::optional<sf::Sprite> coinSprite;
        std::optional<sf::Text> coinText;

        // --- FERTILIZERS UI ---
        std::optional<sf::Sprite> mainFertilizerSprite;
        std::optional<sf::Text> mainFertilizerText;

        bool isFertilizerPopupOpen = false;
        sf::RectangleShape fertilizerPopupBg;

        std::optional<sf::Sprite> regFertSprite;
        std::optional<sf::Text> regFertText;

        std::optional<sf::Sprite> medFertSprite;
        std::optional<sf::Text> medFertText;

        std::optional<sf::Sprite> bestFertSprite;
        std::optional<sf::Text> bestFertText;

        // Pomocnicza metoda do aktualizacji tekstury g³ównej ikony
        void updateMainFertilizerIcon();


        struct Firefly {
            sf::Vector2f position;
            float speed = 0.0f, alpha = 0.0f, lifetime = 0.0f, maxLifetime = 0.0f, size = 0.0f, swayOffset = 0.0f;
        };
        std::vector<Firefly> fireflies;

        void initUI();
        void loadSelectedCharacter();
        void initFireflies();
        void updateFireflies(float dt);

        // Helpers
        void refreshTexts();

    public:
        LobbyState(game::Game* game);
        ~LobbyState() override = default;

        StateType getType() const override { return StateType::Lobby; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}