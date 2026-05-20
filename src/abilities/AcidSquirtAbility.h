#pragma once
#include "Ability.h"
#include "../projectiles/Bullet.h"
#include <vector>
#include <memory>

namespace game::entities { class Entity; }

namespace game::components
{
    class AcidSquirtAbility : public Ability
    {
    private:
        std::vector<game::components::Bullet>* bullets;
        game::entities::Entity* entity;
        std::shared_ptr<sf::Texture> projTexture;

        float baseCooldown = 0.6f;
        float currentTimer = 0.0f;

    public:
        AcidSquirtAbility(std::vector<game::components::Bullet>& bulletsRef, game::entities::Entity* e, std::string texturePath);
        void update(float dt) override;
        void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
    };
}