// --- ShotgunAbility.cpp ---

#include "ShotgunAbility.h"

#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../components/StatsComponent.h"
#include "../components/PlayerInputComponent.h"
#include "../core/ArenaContext.h"

#include <cmath>
#include <numbers>

namespace game::components
{
    namespace
    {
        constexpr float MIN_AIM_LENGTH = 0.001f;

        constexpr float PELLET_SPAWN_OFFSET = 30.0f;
        constexpr float SHOOTER_VELOCITY_FACTOR = 0.5f;

        constexpr float PELLET_RADIUS = 6.2f;

        const sf::Color PELLET_COLOR(
            45,
            25,
            15);
    }

    ShotgunAbility::ShotgunAbility(
        game::ArenaContext* context,
        game::entities::Entity* owner)
        : context_(context),
        owner_(owner)
    {
    }

    void ShotgunAbility::update(
        float deltaTime)
    {
        if (cooldownTimer_ > 0.0f)
        {
            cooldownTimer_ -= deltaTime;
        }
    }

    void ShotgunAbility::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (isOnCooldown())
        {
            return;
        }

        auto* stats = owner_->getComponent<StatsComponent>();
        if (stats && owner_->getComponent<PlayerInputComponent>())
        {
            if (stats->getMana() < manaCost_) return;
        }

        const sf::Vector2f direction =
            calculateDirection(
                origin,
                targetPosition);

        if (direction == sf::Vector2f(0.f, 0.f))
        {
            return;
        }

        spawnPellets(
            origin,
            direction,
            ownerVelocity);

        resetCooldown();

        // Odejmowanie many
        if (stats && owner_->getComponent<PlayerInputComponent>()) {
            stats->consumeMana(manaCost_);
        }
    }

    sf::Vector2f ShotgunAbility::calculateDirection(
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

    void ShotgunAbility::spawnPellets(
        const sf::Vector2f& origin,
        const sf::Vector2f& direction,
        const sf::Vector2f& ownerVelocity)
    {
        if (!context_) return;

        const float baseAngle =
            std::atan2(
                direction.y,
                direction.x);

        const float spreadRadians =
            spreadAngleDegrees_ *
            (
                std::numbers::pi_v<float> /
                180.0f
                );

        const float halfSpread =
            spreadRadians / 2.0f;

        for (int pelletIndex = 0;
            pelletIndex < pelletCount_;
            ++pelletIndex)
        {
            const float angleOffset =
                (pelletCount_ > 1)
                ? (
                    -halfSpread +
                    (
                        spreadRadians *
                        static_cast<float>(
                            pelletIndex) /
                        (pelletCount_ - 1)
                        )
                    )
                : 0.0f;

            const float finalAngle =
                baseAngle + angleOffset;

            const sf::Vector2f pelletDirection(
                std::cos(finalAngle),
                std::sin(finalAngle));

            const sf::Vector2f spawnPosition =
                origin +
                (
                    pelletDirection *
                    PELLET_SPAWN_OFFSET
                    );

            // Budujemy prawdziwa encje ECS
            auto pelletEntity = std::make_unique<game::entities::Entity>();

            // 1. Nadpisujemy jej pozycje w automatycznie wygenerowanym TransformComponent
            if (auto* transform = pelletEntity->getComponent<game::components::TransformComponent>())
            {
                transform->position = spawnPosition;
            }

            // 2. Tworzymy pocisk i dorzucamy go do encji
            auto projectile = std::make_unique<game::components::ProjectileComponent>(
                spawnPosition,
                pelletDirection);

            projectile->addVelocity(
                ownerVelocity *
                SHOOTER_VELOCITY_FACTOR);

            projectile->setAppearance(
                PELLET_RADIUS,
                PELLET_COLOR);

            pelletEntity->addComponent(std::move(projectile));

            // 3. Rejestrujemy gotow¹ encjê w glownym swiecie
            context_->spawnEntity(std::move(pelletEntity));
        }
    }

    bool ShotgunAbility::isOnCooldown() const
    {
        return cooldownTimer_ > 0.0f;
    }

    void ShotgunAbility::resetCooldown()
    {
        cooldownTimer_ = cooldown_;
    }
}