#include "RindRollAbility.h"
#include "../entities/Entity.h"
#include <cmath>

namespace game::components
{
    RindRollAbility::RindRollAbility(game::entities::Entity* targetEntity, float customDuration, float customSpeed)
        : entity(targetEntity), rollDuration(customDuration), rollSpeed(customSpeed) {
    }

    void RindRollAbility::update(float dt)
    {
        if (currentTimer > 0.0f) currentTimer -= dt;
    }

    void RindRollAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity)
    {
        if (currentTimer <= 0.0f && entity != nullptr)
        {
            sf::Vector2f aimDir = targetWorldPos - startPos;
            float length = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);

            if (length > 0.001f)
            {
                aimDir /= length;

                entity->velocity = aimDir * rollSpeed;
                entity->overrideSpeedLimit = rollSpeed;
                entity->isRolling = true;
                entity->actionTimer = rollDuration;

                currentTimer = cooldown;
            }
        }
    }
}