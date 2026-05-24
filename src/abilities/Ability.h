#pragma once

#include <SFML/Graphics.hpp>

namespace game::components
{
    class Ability
    {
    public:
        virtual ~Ability() = default;

        virtual void update(float deltaTime) = 0;

        virtual void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) = 0;
    };
}