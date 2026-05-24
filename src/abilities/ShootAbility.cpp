#include "ShootAbility.h"

#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float MIN_AIM_LENGTH = 0.001f;
    }

    ShootAbility::ShootAbility(
        std::vector<Bullet>& bulletContainer)
        : bullets_(bulletContainer)
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
        bullets_.emplace_back(
            origin,
            direction);

        bullets_.back()
            .addVelocity(ownerVelocity);
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