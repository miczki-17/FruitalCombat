// --- PlayingState.h ---


#pragma once
#include "../abilities/Ability.h"
#include "../core/Game.h"
#include "../projectiles/Bullet.h"
#include "../entities/Entity.h"
#include "State.h"
#include <memory>
#include <vector>
#include <optional>

#include "../factories/MutantFactory.h"
#include "../systems/EvolutionManager.h"
#include "../components/DNAComponent.h"
#include "../components/ColliderComponent.h"
#include "../components/StatsComponent.h"


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


        // --- USER INTERFACE (HUD) ---
        sf::RectangleShape hpBarBg;
        sf::RectangleShape hpBarFill;
        std::optional<sf::Text> hpText;
        std::optional<sf::Text> waveText;

        sf::CircleShape biomassIcon;
        std::optional<sf::Text> biomassText;

        void renderHUD(sf::RenderWindow& window);


        std::vector<std::unique_ptr<game::entities::Entity>> enemies;

        std::unique_ptr<game::factories::MutantFactory> mutantFactory;
        std::unique_ptr<game::systems::EvolutionManager> evolutionManager;

        // walk particles
        float playerDustSpawnTimer = 0.0f;


    public:
        PlayingState(game::Game* game);

        StateType getType() const override;
        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}