#pragma once
#include <SFML/Graphics.hpp>

namespace game::components
{
    // Base abstract class representing a character action or weapon
    class Ability
    {
    public:
        virtual ~Ability() = default;

        // Progress cooldowns or internal timers
        virtual void update(float dt) = 0;

        // Trigger the active execution of the ability
        virtual void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) = 0;
    };
}