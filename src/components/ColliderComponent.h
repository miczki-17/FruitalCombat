#pragma once

#include "Component.h"

#include <SFML/Graphics.hpp>

namespace game::components
{
    class ColliderComponent final : public Component
    {
    public:
        ColliderComponent(
            const sf::Image& collisionMask,
            float mapScale,
            float collisionRadius = 25.0f);

        void update(float deltaTime) override;

    private:
        const sf::Image& collisionMask_;

        float mapScale_;
        float collisionRadius_;

        bool isWallPixel(
            const sf::Vector2f& worldPosition) const;

        sf::Vector2f calculatePushVector(
            const sf::Vector2f& position,
            int& hitCount) const;

        void resolveWallCollision(
            sf::Vector2f& nextPosition,
            float deltaTime);

        void pushOutsideWalls(
            sf::Vector2f& nextPosition);

        sf::Vector2f normalize(
            const sf::Vector2f& vector) const;

        float vectorLength(
            const sf::Vector2f& vector) const;
    };
}