// --- ShootAbility.cpp ---

#include "ShootAbility.h"

#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../core/ArenaContext.h"

#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float MIN_AIM_LENGTH = 0.001f;
    }

    ShootAbility::ShootAbility(
        game::ArenaContext* context)
        : context_(context)
    {
    }

    void ShootAbility::update(float deltaTime)
    {
        if (cooldownTimer_ > 0.0f)
        {
            cooldownTimer_ -= deltaTime;
        }
    }

    void ShootAbility::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (isOnCooldown())
        {
            return;
        }

        const sf::Vector2f direction =
            calculateDirection(
                origin,
                targetPosition);

        if (direction == sf::Vector2f(0.f, 0.f))
        {
            return;
        }

        spawnProjectile(
            origin,
            direction,
            ownerVelocity);

        resetCooldown();
    }

    sf::Vector2f ShootAbility::calculateDirection(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition) const
    {
        sf::Vector2f direction =
            targetPosition - origin;

        const float length =
            std::sqrt(
                direction.x * direction.x +
                direction.y * direction.y);

        if (length <= MIN_AIM_LENGTH)
        {
            return { 0.f, 0.f };
        }

        return direction / length;
    }

    void ShootAbility::spawnProjectile(
        const sf::Vector2f& origin,
        const sf::Vector2f& direction,
        const sf::Vector2f& ownerVelocity)
    {
        if (!context_) return;

        auto bulletEntity = std::make_unique<game::entities::Entity>();

        if (auto* transform = bulletEntity->getComponent<game::components::TransformComponent>())
        {
            transform->position = origin;
        }

        auto projectile = std::make_unique<game::components::ProjectileComponent>(
            origin,
            direction);

        projectile->addVelocity(ownerVelocity);

        bulletEntity->addComponent(std::move(projectile));

        context_->spawnEntity(std::move(bulletEntity));
    }

    bool ShootAbility::isOnCooldown() const
    {
        return cooldownTimer_ > 0.0f;
    }

    void ShootAbility::resetCooldown()
    {
        cooldownTimer_ = cooldown_;
    }
}