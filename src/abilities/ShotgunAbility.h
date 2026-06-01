// --- ShotgunAbility.h ---

#pragma once

#include "Ability.h"
#include "../components/ProjectileComponent.h"
#include "../core/ArenaContext.h"

#include <vector>

//namespace game { struct Arenacontext; }
//namespace game::entities { class Entity; }

namespace game::components
{
    class ShotgunAbility final : public Ability
    {
    public:
        explicit ShotgunAbility(
            game::ArenaContext* context);

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        game::ArenaContext* context_;

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