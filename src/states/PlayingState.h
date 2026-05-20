// --- PlayingState.h ---


#pragma once
#include "../abilities/Ability.h"
#include "../core/Game.h"
#include "../projectiles/Bullet.h"
#include "../entities/Entity.h"
#include "../core/State.h"
#include <memory>
#include <vector>
#include <optional>

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
        std::unique_ptr<game::entities::Entity> player;

        // Bullets container owned cleanly by the arena environment state
        std::vector<game::components::Bullet> bullets;

        sf::Font uiFont;
        sf::Texture coinIconTexture;
        std::optional<sf::Sprite> coinIconSprite;

        sf::Texture settingsBtnTex;
        std::optional<sf::Sprite> settingsBtnSprite;

        sf::Texture crosshairTex;
        std::optional<sf::Sprite> crosshairSprite;

        void renderHUD(sf::RenderWindow& window);

    public:
        PlayingState(game::Game* game);

        StateType getType() const override;
        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}