#pragma once

#include "Component.h"
#include "../core/Game.h"

namespace game::components
{
    class MovementComponent final
        : public Component
    {
    public:
        MovementComponent(
            game::Game* game,
            float maxSpeed);

        void update(
            float deltaTime) override;

        void setGamePointer(
            game::Game* game) 
        { 
            game_ = game;
        }

    private:
        game::Game* game_;

        float maxSpeed_;

        float acceleration_ =
            2500.0f;

        float activeDrag_ =
            4.0f;

        float stopDrag_ =
            8.0f;

        float turnSpeed_ =
            15.0f;

        sf::Vector2f
            smoothedInput_;

        sf::Vector2f
            readMovementInput() const;

        void updateFacingDirection(
            const sf::Vector2f&
            direction);

        sf::Vector2f normalize(
            const sf::Vector2f&
            vector) const;

        float vectorLength(
            const sf::Vector2f&
            vector) const;

        void applyMovement(
            const sf::Vector2f&
            direction,
            float deltaTime);

        void applyStopping(
            float deltaTime);

        void updateRollingState(
            float deltaTime);

        void limitSpeed(
            float maxSpeed);
    };
}