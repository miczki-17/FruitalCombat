#pragma once

#include "Ability.h"

namespace game { struct ArenaContext; }
namespace game::entities { class Entity; }

namespace game::components
{
    class PoisonExplosionAbility final : public Ability
    {
    public:
        PoisonExplosionAbility(
            game::ArenaContext* context,
            game::entities::Entity* owner,
            float radius,
            float dps,
            float duration,
            const std::string& textureKey,
            std::string sourceName = "Undefined Biomass");

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        game::ArenaContext* context_;
        game::entities::Entity* owner_;

        std::string textureKey_;


        float radius_;
        float dps_;
        float duration_;
    };
}