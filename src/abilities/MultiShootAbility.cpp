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
        float damage)
        : context_(context), owner_(owner), textureKey_(textureKey),
        projectileCount_(projectileCount), burstDelay_(burstDelay),
        projectileScale_(projectileScale), cooldown_(cooldown), damage_(damage)
    {
    }

    void MultiShootAbility::update(float deltaTime)
    {
        // 1. Je?li jeste?my w trakcie strzelania seri?
        if (isBursting_)
        {
            burstTimer_ -= deltaTime;
            if (burstTimer_ <= 0.0f)
            {
                // Aktualizujemy pozycj? startow? -> mozliwosc ruchu miedzy strzalami
                auto* transform = owner_->getComponent<TransformComponent>();
                sf::Vector2f currentOrigin = transform ? transform->position : sf::Vector2f(0.f, 0.f);

                // Strzelamy w zamro?onym kierunku
                spawnProjectile(currentOrigin, burstDirection_, burstOwnerVelocity_);
                projectilesFired_++;

                if (projectilesFired_ >= projectileCount_) {
                    // Seria zako?czona - odpalamy g?ówny cooldown
                    isBursting_ = false;
                    resetCooldown();
                }
                else {
                    // Resetujemy czas do kolejnego strza?u z serii
                    burstTimer_ = burstDelay_;
                }
            }
        }
        // 2. Standardowy cooldown po zako?czeniu serii
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
        // Blokujemy wywo?anie, je?li ?adujemy atak, LUB je?li w?a?nie strzelamy seri?
        if (isOnCooldown() || isBursting_) return;

        auto* stats = owner_->getComponent<StatsComponent>();
        if (stats && owner_->getComponent<PlayerInputComponent>()) {
            if (stats->getMana() < manaCost_) return;
            stats->consumeMana(manaCost_); // P?acimy za ca?? seri? z góry
        }

        burstDirection_ = calculateBaseDirection(origin, targetPosition);
        if (burstDirection_ == sf::Vector2f(0.f, 0.f)) return;

        burstOwnerVelocity_ = ownerVelocity;

        // B?YSKAWICZNA REAKCJA: Wystrzeliwujemy pierwszy pocisk od razu
        spawnProjectile(origin, burstDirection_, burstOwnerVelocity_);
        projectilesFired_ = 1;

        if (projectileCount_ > 1) {
            isBursting_ = true;
            burstTimer_ = burstDelay_; // Ustawiamy timer dla drugiego pocisku
        }
        else {
            resetCooldown(); // Zabezpieczenie: je?li kto? ustawi 1 pocisk w serii
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

        auto projectile = std::make_unique<game::components::ProjectileComponent>(origin, direction);
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