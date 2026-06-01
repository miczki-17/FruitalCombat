// --- AcidPoolUltimate.cpp ---

#include "AcidPoolUltimate.h"

#include "../components/StatsComponent.h"
#include "../components/AoEComponent.h"
#include "../components/LifespanComponent.h"
#include "../components/TransformComponent.h"
#include "../entities/Entity.h"

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
        auto* owner_transform = owner_->getComponent<TransformComponent>();
        if (!owner_transform) return;

        auto aoeEntity = std::make_unique<game::entities::Entity>();

        // 1. Pozycja ka³u¿y
        if (auto* transform = aoeEntity->getComponent<game::components::TransformComponent>()) {
            transform->position = owner_transform->position;
        }

        aoeEntity->addComponent(std::make_unique<game::components::AoEComponent>(
            ACID_POOL_RADIUS,
            ACID_POOL_COLOR,
            0.0f,                       // Bazowy DPS
            true,                       // appliesPoison
            POISON_DAMAGE_PER_SECOND,   // poisonDps
            true,                       // appliesSlow
            SLOW_MULTIPLIER             // slowMult
            ));

        // 3. Dodajemy czas ¿ycia z p³ynnym wygaszaniem (fadeOut = true)
        aoeEntity->addComponent(std::make_unique<game::components::LifespanComponent>(ACID_POOL_DURATION, true));

        // 4. Wrzucamy do systemu!
        arenaContext_->spawnEntity(std::move(aoeEntity));
    }
}