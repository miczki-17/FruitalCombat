// --- AcidPoolUltimate.h ---

#pragma once

#include "Ability.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"

namespace game::components
{
    class AcidPoolUltimate final : public Ability
    {
    public:
        AcidPoolUltimate(
            game::entities::Entity* owner,
            game::ArenaContext* arenaContext,
            std::string sourceName = "Undefined Biomass");

        void update(float deltaTime) override {}

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        game::entities::Entity* owner_;
        game::ArenaContext* arenaContext_;

        void applyBuff() const;
        void spawnAcidPool() const;
    };
}