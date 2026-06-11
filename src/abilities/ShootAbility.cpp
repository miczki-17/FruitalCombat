// --- ShootAbility.cpp ---

#include "ShootAbility.h"

#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../components/playerInputComponent.h"
#include "../components/SpriteComponent.h"
#include "../core/ResourceManager.h"
#include "../core/ArenaContext.h"

#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float MIN_AIM_LENGTH = 0.001f;
        constexpr float PI = 3.14159265359f;
    }

    ShootAbility::ShootAbility(
        game::ArenaContext* context,
        game::entities::Entity* owner,
        const std::string& textureKey,
        float projectileScale,
        float cooldown,
        float damage)
        : context_(context),
        owner_(owner),
        textureKey_(textureKey),
        projectileScale_(projectileScale),
        cooldown_(cooldown),
        damage_(damage)
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
        if (isOnCooldown()) return;

        // SPRAWDZENIE I KONSUMPCJA MANY
        auto* stats = owner_->getComponent<StatsComponent>();
        if (stats) {
            if (owner_->getComponent<PlayerInputComponent>() && stats->getMana() < manaCost_) {
                return;
            }
        }

        const sf::Vector2f direction = calculateDirection(origin, targetPosition);
        if (direction == sf::Vector2f(0.f, 0.f)) return;

        spawnProjectile(origin, direction, ownerVelocity);
        resetCooldown();

        // Odejmowanie many
        if (stats && owner_->getComponent<PlayerInputComponent>()) {
            stats->consumeMana(manaCost_);
        }
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

        // TRANSFORM POCISKU
        bulletEntity->addComponent(std::make_unique<game::components::TransformComponent>(origin));

        auto projectile = std::make_unique<game::components::ProjectileComponent>(
            origin,
            direction);

        projectile->addVelocity(ownerVelocity);

        // Przekazanie obra?e? na pocisk
        projectile->setDamage(damage_);

        bool isPlayer = (owner_->getComponent<game::components::PlayerInputComponent>() != nullptr);
        projectile->setFriendly(isPlayer);

        // NAKLADANIE TEXTURY
        if (!textureKey_.empty())
        {
            auto tex = game::core::ResourceManager::get().getTextureShared(textureKey_);
            if (tex)
            {
                auto size = tex->getSize();
                projectile->setAnimation(
                    tex, 1, 1.0f, { static_cast<int>(size.x), static_cast<int>(size.y) }
                );
                projectile->setSpriteScale(projectileScale_, projectileScale_);
            }
        }

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