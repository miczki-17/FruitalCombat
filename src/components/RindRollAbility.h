#pragma once
#include "Ability.h"
#include "../entities/player/Player.h"

namespace game::components
{
    class RindRollAbility : public Ability
    {
    private:
        game::entities::Player* player;
        float cooldown = 5.0f;
        float currentTimer = 0.0f;

        // Rolling parameters dynamically set from JSON configuration
        float rollDuration = 1.5f;
        float rollSpeed = 80.0f;

    public:
        // Constructor updated to support dynamic property injection from the factory
        RindRollAbility(game::entities::Player* p, float customDuration = 1.5f, float customSpeed = 80.0f);

        void update(float dt) override;
        void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
    };
}