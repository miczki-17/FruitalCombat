// --- ShootAbility.h ---

#pragma once

#include "Ability.h"
#include "../components/ProjectileComponent.h"

#include <vector>

namespace game { struct ArenaContext; }
namespace game::entities { class Entity; }

namespace game::components
{
    class ShootAbility final : public Ability
    {
    public:
        explicit ShootAbility(
            game::ArenaContext* context,
            game::entities::Entity* owner);

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        game::ArenaContext* context_;
        game::entities::Entity* owner_;

        // MANA cost
        float manaCost_ = 15.0f;

        float cooldown_ = 0.2f;
        float cooldownTimer_ = 0.0f;

        bool isOnCooldown() const;
        void resetCooldown();

        sf::Vector2f calculateDirection(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition) const;

        void spawnProjectile(
            const sf::Vector2f& origin,
            const sf::Vector2f& direction,
            const sf::Vector2f& ownerVelocity);
    };
}