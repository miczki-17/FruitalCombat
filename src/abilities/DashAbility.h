#pragma once
#include "Ability.h"

namespace game::entities { class Entity; }

namespace game::components
{
    class DashAbility : public Ability
    {
    private:
        game::entities::Entity* entity;
        float cooldown = 2.0f;
        float currentTimer = 0.0f;
        float dashForce = 1200.0f;

    public:
        DashAbility(game::entities::Entity* targetEntity);
        void update(float dt) override;
        void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
    };
}