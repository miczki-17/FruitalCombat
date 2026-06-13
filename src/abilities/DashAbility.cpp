// --- DashAbility.cpp ---

#include "DashAbility.h"

#include "../entities/Entity.h"
#include "../components/ProjectileComponent.h"
#include "../components/TransformComponent.h"
#include "../components/PlayerInputComponent.h"

#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float MIN_MOVEMENT_SPEED = 10.0f;
        constexpr float MIN_VECTOR_LENGTH = 0.001f;
        constexpr float ACTION_UNLOCK_DURATION = 0.15f;

        const sf::Vector2f DEFAULT_DASH_DIRECTION(
            1.0f,
            0.0f);
    }

    DashAbility::DashAbility(
        game::entities::Entity* owner,
        std::string sourceName)
        : owner_(owner),
        Ability(std::move(sourceName))
    {
    }

    void DashAbility::update(float deltaTime)
    {
        if (cooldownTimer_ > 0.0f)
        {
            cooldownTimer_ -= deltaTime;
        }
    }

    void DashAbility::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (!owner_ || isOnCooldown())
        {
            return;
        }

        // SPRAWDZENIE I KONSUMPCJA MANY
        auto* stats = owner_->getComponent<StatsComponent>();
        if (stats) {
            if (owner_->getComponent<PlayerInputComponent>() && stats->getMana() < manaCost_) {
                return;
            }
        }

        auto* owner_transform = owner_->getComponent<TransformComponent>();

        const sf::Vector2f dashDirection =
            calculateDashDirection(
                origin,
                targetPosition,
                ownerVelocity);

        owner_transform->velocity +=
            (dashDirection * dashForce_);

        owner_transform->actionTimer =
            ACTION_UNLOCK_DURATION;

        cooldownTimer_ = cooldown_;

        // Odejmowanie many
        if (stats && owner_->getComponent<PlayerInputComponent>()) {
            stats->consumeMana(manaCost_);
        }
    }

    bool DashAbility::isOnCooldown() const
    {
        return cooldownTimer_ > 0.0f;
    }

    sf::Vector2f DashAbility::calculateDashDirection(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity) const
    {
        const float movementSpeed =
            std::sqrt(
                ownerVelocity.x * ownerVelocity.x +
                ownerVelocity.y * ownerVelocity.y);

        if (movementSpeed > MIN_MOVEMENT_SPEED)
        {
            return normalize(ownerVelocity);
        }

        const sf::Vector2f aimDirection =
            targetPosition - origin;

        const float aimLength =
            std::sqrt(
                aimDirection.x * aimDirection.x +
                aimDirection.y * aimDirection.y);

        if (aimLength > MIN_VECTOR_LENGTH)
        {
            return normalize(aimDirection);
        }

        return DEFAULT_DASH_DIRECTION;
    }

    sf::Vector2f DashAbility::normalize(
        const sf::Vector2f& vector) const
    {
        const float length =
            std::sqrt(
                vector.x * vector.x +
                vector.y * vector.y);

        if (length <= MIN_VECTOR_LENGTH)
        {
            return DEFAULT_DASH_DIRECTION;
        }

        return vector / length;
    }
}