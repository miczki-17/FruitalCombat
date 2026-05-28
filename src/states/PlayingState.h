// --- PlayingState.h ---

#pragma once
#include "State.h"
#include "../core/Game.h"
#include "../core/GameWorld.h"
#include <memory>
#include <optional>
#include <SFML/Graphics.hpp>

namespace game::states
{
    class PlayingState : public State
    {
    private:
        sf::Texture mapTexture;
        std::optional<sf::Sprite> mapSprite;
        sf::Vector2f mapLimits;
        sf::Image collisionMask;
        float mapScale = 1.5f;

        sf::View cameraView;

        // HUD & UI
        sf::Font uiFont;
        sf::Texture coinIconTexture, settingsBtnTex, crosshairTex;
        std::optional<sf::Sprite> coinIconSprite, settingsBtnSprite, crosshairSprite;
        sf::RectangleShape hpBarBg, hpBarFill;
        std::optional<sf::Text> hpText, waveText, biomassText;
        sf::CircleShape biomassIcon;

        float shakeIntensity = 0.0f;

        // --- All logic closed in one pointer ---
        std::unique_ptr<game::core::GameWorld> world;

        void initHUD();
        void updateHUD();
        void updateCamera(float dt);
        void handleUIHover();
        void renderHUD(sf::RenderWindow& window);



        // Memory clear
        std::string loadedSplashKey_ = "";

    public:
        PlayingState(game::Game* game);
        ~PlayingState() override;

        StateType getType() const override;
        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}