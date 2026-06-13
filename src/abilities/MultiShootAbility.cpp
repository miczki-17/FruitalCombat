// --- MultiShootAbility.cpp ---

#include "MultiShootAbility.h"
#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../components/PlayerInputComponent.h"
#include "../components/ProjectileComponent.h"
#include "../components/StatsComponent.h"
#include "../core/ResourceManager.h"
#include "../core/ArenaContext.h"
#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float MIN_AIM_LENGTH = 0.001f;
    }

    MultiShootAbility::MultiShootAbility(
        game::ArenaContext* context,
        game::entities::Entity* owner,
        const std::string& textureKey,
        int projectileCount,
        float burstDelay,
        float projectileScale,
        float cooldown,
        float damage,
        std::string sourceName)
        : Ability(std::move(sourceName)),
        context_(context), owner_(owner), textureKey_(textureKey),
        projectileCount_(projectileCount), burstDelay_(burstDelay),
        projectileScale_(projectileScale), cooldown_(cooldown), damage_(damage)
    {
    }

    void MultiShootAbility::update(float deltaTime)
    {
        // 1. Jeœli jesteœmy w trakcie strzelania seri¹
        if (isBursting_)
        {
            burstTimer_ -= deltaTime;
            if (burstTimer_ <= 0.0f)
            {
                auto* transform = owner_->getComponent<TransformComponent>();
                sf::Vector2f currentOrigin = transform ? transform->position : sf::Vector2f(0.f, 0.f);

                spawnProjectile(currentOrigin, burstDirection_, burstOwnerVelocity_);
                projectilesFired_++;

                if (projectilesFired_ >= projectileCount_) {
                    isBursting_ = false;
                    resetCooldown();
                }
                else {
                    burstTimer_ = burstDelay_;
                }
            }
        }
        else
        {
            if (cooldownTimer_ > 0.0f) cooldownTimer_ -= deltaTime;
        }
    }

    void MultiShootAbility::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (isOnCooldown() || isBursting_) return;

        auto* stats = owner_->getComponent<StatsComponent>();
        if (stats && owner_->getComponent<PlayerInputComponent>()) {
            if (stats->getMana() < manaCost_) return;
            stats->consumeMana(manaCost_); // P³acimy za ca³¹ seriê z góry
        }

        burstDirection_ = calculateBaseDirection(origin, targetPosition);
        if (burstDirection_ == sf::Vector2f(0.f, 0.f)) return;

        burstOwnerVelocity_ = ownerVelocity;

        spawnProjectile(origin, burstDirection_, burstOwnerVelocity_);
        projectilesFired_ = 1;

        if (projectileCount_ > 1) {
            isBursting_ = true;
            burstTimer_ = burstDelay_;
        }
        else {
            resetCooldown();
        }
    }

    sf::Vector2f MultiShootAbility::calculateBaseDirection(const sf::Vector2f& origin, const sf::Vector2f& targetPosition) const
    {
        sf::Vector2f dir = targetPosition - origin;
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length <= MIN_AIM_LENGTH) return { 0.f, 0.f };
        return dir / length;
    }

    void MultiShootAbility::spawnProjectile(
        const sf::Vector2f& origin,
        const sf::Vector2f& direction,
        const sf::Vector2f& ownerVelocity)
    {
        if (!context_) return;

        auto bulletEntity = std::make_unique<game::entities::Entity>();
        bulletEntity->addComponent(std::make_unique<game::components::TransformComponent>(origin));

        auto projectile = std::make_unique<game::components::ProjectileComponent>(origin, direction, sourceName_);
        projectile->addVelocity(ownerVelocity);
        projectile->setDamage(damage_);

        bool isPlayer = (owner_->getComponent<game::components::PlayerInputComponent>() != nullptr);
        projectile->setFriendly(isPlayer);

        if (!textureKey_.empty())
        {
            auto tex = game::core::ResourceManager::get().getTextureShared(textureKey_);
            if (tex) {
                auto size = tex->getSize();
                projectile->setAnimation(tex, 1, 1.0f, { static_cast<int>(size.x), static_cast<int>(size.y) });
                projectile->setSpriteScale(projectileScale_, projectileScale_);
            }
        }

        bulletEntity->addComponent(std::move(projectile));
        context_->spawnEntity(std::move(bulletEntity));
    }

    bool MultiShootAbility::isOnCooldown() const { return cooldownTimer_ > 0.0f; }
    void MultiShootAbility::resetCooldown() { cooldownTimer_ = cooldown_; }
}