#pragma once

#include "Ability.h"

namespace game::entities
{
    class Entity;
}

namespace game::components
{
    class DashAbility final : public Ability
    {
    public:
        explicit DashAbility(game::entities::Entity* owner);

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        game::entities::Entity* owner_;

        float cooldown_ = 2.0f;
        float cooldownTimer_ = 0.0f;
        float dashForce_ = 1200.0f;

        bool isOnCooldown() const;

        sf::Vector2f calculateDashDirection(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) const;

        sf::Vector2f normalize(
            const sf::Vector2f& vector) const;
    };
}