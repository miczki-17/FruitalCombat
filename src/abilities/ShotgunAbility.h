// --- ShotgunAbility.h ---

#pragma once

#include "Ability.h"
#include "../projectiles/Bullet.h"

#include <vector>

namespace game::components
{
    class ShotgunAbility final : public Ability
    {
    public:
        explicit ShotgunAbility(
            std::vector<Bullet>& bulletContainer);

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        std::vector<Bullet>& bullets_;

        float cooldown_ = 0.75f;
        float cooldownTimer_ = 0.0f;

        int pelletCount_ = 2;
        float spreadAngleDegrees_ = 7.5f;

        bool isOnCooldown() const;
        void resetCooldown();

        sf::Vector2f calculateDirection(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition) const;

        void spawnPellets(
            const sf::Vector2f& origin,
            const sf::Vector2f& direction,
            const sf::Vector2f& ownerVelocity);
    };
}