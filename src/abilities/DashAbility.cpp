#include "DashAbility.h"
#include "../entities/Entity.h"
#include <cmath>

namespace game::components
{
    DashAbility::DashAbility(game::entities::Entity* targetEntity) : entity(targetEntity) {}

    void DashAbility::update(float dt)
    {
        if (currentTimer > 0.0f) currentTimer -= dt;
    }

    void DashAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity)
    {
        if (currentTimer <= 0.0f && entity != nullptr)
        {
            sf::Vector2f dashDir = shooterVelocity;
            float currentSpeed = std::sqrt(dashDir.x * dashDir.x + dashDir.y * dashDir.y);

            if (currentSpeed > 10.0f) dashDir /= currentSpeed;
            else
            {
                dashDir = targetWorldPos - startPos;
                float aimLength = std::sqrt(dashDir.x * dashDir.x + dashDir.y * dashDir.y);
                if (aimLength > 0.001f) dashDir /= aimLength;
                else dashDir = { 1.0f, 0.0f };
            }

            entity->velocity += (dashDir * dashForce);
            entity->actionTimer = 0.15f; // Uncap velocity czasowo
            currentTimer = cooldown;
        }
    }
}