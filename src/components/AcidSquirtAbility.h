#pragma once
#include "Ability.h"
#include "Bullet.h"
#include "../entities/player/Player.h"
#include <vector>
#include <memory>

namespace game::components
{
    class AcidSquirtAbility : public Ability
    {
    private:
        std::vector<game::components::Bullet>& bullets;
        game::entities::Player* player;

		// projectile texture
        std::shared_ptr<sf::Texture> projTexture;

        float baseCooldown = 0.6f;
        float currentTimer = 0.0f;

    public:
        AcidSquirtAbility(std::vector<game::components::Bullet>& bulletsRef, game::entities::Player* p, std::string texturePath);

        void update(float dt) override;
        void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
    };
}