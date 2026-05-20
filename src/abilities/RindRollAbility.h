#pragma once
#include "Ability.h"

namespace game::entities { class Entity; }

namespace game::components
{
    class RindRollAbility : public Ability
    {
    private:
        game::entities::Entity* entity;
        float cooldown = 5.0f;
        float currentTimer = 0.0f;
        float rollDuration = 1.5f;
        float rollSpeed = 80.0f;

    public:
        RindRollAbility(game::entities::Entity* targetEntity, float customDuration = 1.5f, float customSpeed = 80.0f);
        void update(float dt) override;
        void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
    };
}