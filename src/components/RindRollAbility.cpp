#include "RindRollAbility.h"
#include <cmath>

namespace game::components
{
    RindRollAbility::RindRollAbility(game::entities::Player* p, float customDuration, float customSpeed)
        : player(p), rollDuration(customDuration), rollSpeed(customSpeed)
    {
    }

    void RindRollAbility::update(float dt)
    {
        if (currentTimer > 0.0f) currentTimer -= dt;
    }

    void RindRollAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity)
    {
        if (currentTimer <= 0.0f && player != nullptr)
        {
            sf::Vector2f aimDir = targetWorldPos - startPos;
            float length = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);

            if (length > 0.001f)
            {
                aimDir /= length;

                // Odpalamy sterowalne toczenie (Drift) z parametrami z JSON-a
                player->startRoll(aimDir, rollSpeed, rollDuration);

                currentTimer = cooldown;
            }
        }
    }
}