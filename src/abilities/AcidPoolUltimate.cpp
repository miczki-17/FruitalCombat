#include "AcidPoolUltimate.h"

#include "../components/StatsComponent.h"

namespace game::components
{
    namespace
    {
        constexpr float SPEED_BUFF_DURATION = 5.0f;
        constexpr float SPEED_MULTIPLIER = 2.0f;

        constexpr float ACID_POOL_RADIUS = 250.0f;
        constexpr float ACID_POOL_DURATION = 6.0f;

        constexpr float POISON_DAMAGE_PER_SECOND = 40.0f;
        constexpr float SLOW_MULTIPLIER = 0.3f;

        constexpr sf::Color ACID_POOL_COLOR(150, 255, 50, 180);
    }

    AcidPoolUltimate::AcidPoolUltimate(
        game::entities::Entity* owner,
        game::ArenaContext* arenaContext)
        : owner_(owner),
        arenaContext_(arenaContext)
    {
    }

    void AcidPoolUltimate::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (!owner_ || !arenaContext_)
        {
            return;
        }

        applyBuff();
        spawnAcidPool();
    }

    void AcidPoolUltimate::applyBuff() const
    {
        auto* stats = owner_->getComponent<StatsComponent>();

        if (!stats)
        {
            return;
        }

        stats->addEffect(
            StatusType::SpeedBuff,
            SPEED_BUFF_DURATION,
            SPEED_MULTIPLIER);
    }

    void AcidPoolUltimate::spawnAcidPool() const
    {
        AoEZone acidPool;

        acidPool.radius = ACID_POOL_RADIUS;
        acidPool.dps = 0.0f;

        acidPool.appliesPoison = true;
        acidPool.poisonDps = POISON_DAMAGE_PER_SECOND;

        acidPool.appliesSlow = true;
        acidPool.slowMultiplier = SLOW_MULTIPLIER;

        acidPool.lifetime = ACID_POOL_DURATION;
        acidPool.maxLifetime = ACID_POOL_DURATION;

        acidPool.shape.setRadius(acidPool.radius);
        acidPool.shape.setOrigin(
            { acidPool.radius, acidPool.radius });

        acidPool.shape.setPosition(owner_->position);
        acidPool.shape.setFillColor(ACID_POOL_COLOR);

        arenaContext_->zones.push_back(acidPool);
    }
}