// --- AcidSquirtAbility.cpp ---

#include "AcidSquirtAbility.h"

#include "../components/StatsComponent.h"
#include "../components/ProjectileComponent.h"
#include "../components/PlayerInputComponent.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"

#include <cmath>
#include <iostream>

namespace game::components
{
    namespace
    {
        constexpr float PROJECTILE_SPREAD_ANGLE = 0.35f;
        constexpr float PROJECTILE_SPEED = 380.0f;

        constexpr float PROJECTILE_RADIUS = 14.0f;

        constexpr int ANIMATION_FRAMES = 4;
        constexpr float ANIMATION_FRAME_DURATION = 0.1f;

        const sf::Color PROJECTILE_COLOR(100, 255, 0, 240);
        const sf::Vector2i ANIMATION_FRAME_SIZE(32, 32);
    }

    AcidSquirtAbility::AcidSquirtAbility(
        game::ArenaContext* context,
        game::entities::Entity* owner,
        const std::string& texturePath,
        const std::string& splashKeyBase,
        bool isFriendly)
        : context_(context),
        owner_(owner),
        projectileTexture_(std::make_shared<sf::Texture>()),
        splashKeyBase_(splashKeyBase),
        isFriendly_(isFriendly)
    {
        if (!projectileTexture_->loadFromFile(texturePath))
        {
            std::cerr
                << "[ERROR] Could not load texture: "
                << texturePath
                << '\n';
        }
    }

    void AcidSquirtAbility::update(float deltaTime)
    {
        if (cooldownTimer_ > 0.0f)
        {
            cooldownTimer_ -= deltaTime;
        }
    }

    void AcidSquirtAbility::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (context_ == nullptr || isOnCooldown())
        {
            return;
        }

        // 1. SPRAWDZENIE MANY
        auto* stats = owner_->getComponent<StatsComponent>();
        bool isPlayer = owner_->getComponent<PlayerInputComponent>() != nullptr;

        if (stats && isPlayer) {
            if (stats->getMana() < manaCost_) {
                return;
            }
        }

        const float attackSpeedModifier =
            getAttackSpeedModifier();

        const int projectileCount =
            1 + getBonusProjectileCount();

        const sf::Vector2f aimDirection =
            targetPosition - origin;

        const float distanceToTarget =
            std::sqrt(
                aimDirection.x * aimDirection.x +
                aimDirection.y * aimDirection.y);

        const float baseAngle =
            std::atan2(aimDirection.y, aimDirection.x);

        const float startAngle =
            baseAngle -
            (
                PROJECTILE_SPREAD_ANGLE *
                (projectileCount - 1) /
                2.0f
                );

        for (int projectileIndex = 0;
            projectileIndex < projectileCount;
            ++projectileIndex)
        {
            const float currentAngle =
                startAngle +
                (projectileIndex *
                    PROJECTILE_SPREAD_ANGLE);

            const sf::Vector2f direction(
                std::cos(currentAngle),
                std::sin(currentAngle));

            const sf::Vector2f projectileTarget =
                origin +
                (direction * distanceToTarget);

            spawnProjectile(
                origin,
                projectileTarget);
        }

        // 2. KONSUMPCJA MANY
        if (stats && isPlayer) {
            stats->consumeMana(manaCost_);
        }

        resetCooldown(attackSpeedModifier);
    }

    void AcidSquirtAbility::spawnProjectile(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition)
    {
        if (!context_) return;

        // TWORZYMY NOW¥, CZYST¥ ENCJÊ ECS
        auto bulletEntity = std::make_unique<game::entities::Entity>();

        // 1. TransformComponent (Pozycja) - nadpisujemy domyœlny tworzony w konstruktorze Entity
        if (auto* transform = bulletEntity->getComponent<game::components::TransformComponent>())
        {
            transform->position = origin;
        }

        // 2. Tworzymy pocisk (Logikê uderzeniow¹)
        // Podajemy {0,0} jako kierunek, bo setupParabolic zaraz i tak to nadpisze
        auto projectile = std::make_unique<game::components::ProjectileComponent>(origin, sf::Vector2f(0.f, 0.f));

        projectile->setAppearance(PROJECTILE_RADIUS, PROJECTILE_COLOR);
        projectile->setStatusEffect(game::components::StatusEffect::Poison);
        projectile->setWobble(false, false);
        projectile->setupParabolic(origin, targetPosition, PROJECTILE_SPEED);
        projectile->setFriendly(isFriendly_);
        projectile->setSplashKeyBase(splashKeyBase_);

        if (projectileTexture_->getSize().x > 0)
        {
            projectile->setAnimation(
                projectileTexture_,
                ANIMATION_FRAMES,
                ANIMATION_FRAME_DURATION,
                ANIMATION_FRAME_SIZE);
        }

        // Dodajemy logikê pocisku do encji
        bulletEntity->addComponent(std::move(projectile));

        context_->spawnEntity(std::move(bulletEntity));
    }

    float AcidSquirtAbility::getAttackSpeedModifier() const
    {
        if (!owner_)
        {
            return 1.0f;
        }

        auto* stats =
            owner_->getComponent<StatsComponent>();

        if (!stats)
        {
            return 1.0f;
        }

        return stats->getAttackSpeed();
    }

    int AcidSquirtAbility::getBonusProjectileCount() const
    {
        if (!owner_)
        {
            return 0;
        }

        auto* stats =
            owner_->getComponent<StatsComponent>();

        if (!stats)
        {
            return 0;
        }

        return stats->getBonusProjectiles();
    }

    bool AcidSquirtAbility::isOnCooldown() const
    {
        return cooldownTimer_ > 0.0f;
    }

    void AcidSquirtAbility::resetCooldown(
        float attackSpeedModifier)
    {
        cooldownTimer_ =
            baseCooldown_ / attackSpeedModifier;
    }
}