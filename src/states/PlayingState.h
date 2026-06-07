// --- PlayingState.h ---

#pragma once
#include "State.h"
#include "../core/Game.h"
#include "../core/GameWorld.h"
#include <memory>
#include <optional>
#include <string>
#include <SFML/Graphics.hpp>

namespace game::states
{
    class PlayingState : public State
    {
    private:
        // --- MAP & CAMERA ---
        sf::Texture mapTexture;
        std::optional<sf::Sprite> mapSprite;
        sf::Vector2f mapLimits;
        sf::Image collisionMask;
        float mapScale = 1.5f;

        sf::View cameraView;
        float shakeIntensity = 0.0f;

        // --- HUD & UI ---
        std::optional<sf::Sprite> settingsBtnSprite;
        std::optional<sf::Sprite> crosshairSprite;
        std::optional<sf::Sprite> biomassSprite;

        std::optional<sf::Sprite> hpBarFrameSprite;
        std::optional<sf::Sprite> hpBarFillSprite;

        std::optional<sf::Sprite> manaBarFrameSprite;
        std::optional<sf::Sprite> manaBarFillSprite;

        std::optional<sf::Text> manaText;
        std::optional<sf::Text> hpText;
        std::optional<sf::Text> waveText;
        std::optional<sf::Text> biomassText;

        // Fertilizer
        std::optional<sf::Sprite> activeFertilizerSprite;
        std::optional<sf::Text> activeFertilizerText;

        // --- GAME WORLD ---
        std::unique_ptr<game::core::GameWorld> world;

        // --- HELPERS & RESOURCES ---
        std::string loadedSplashKey_ = "";

        void loadCharacterAssets();
        void loadEnemyAssets();
        void loadMapAssets();
        void loadUpgrades();

        void initHUD();
        void updateHUD();
        void updateCamera(float dt);
        void renderHUD(sf::RenderWindow& window);

    public:
        PlayingState(game::Game* game);
        ~PlayingState() override;

        StateType getType() const override { return StateType::Playing; }
        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}