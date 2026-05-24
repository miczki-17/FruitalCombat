#include "MovementComponent.h"

#include "../entities/Entity.h"

#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float
            MIN_VECTOR_LENGTH =
            0.001f;

        constexpr float
            STOP_THRESHOLD =
            10.0f;

        constexpr float
            LOOK_THRESHOLD =
            0.1f;

        constexpr float
            INPUT_THRESHOLD =
            0.05f;
    }

    MovementComponent::
        MovementComponent(
            game::Game* game,
            float maxSpeed)
        : game_(game),
        maxSpeed_(
            maxSpeed)
    {
    }

    void MovementComponent::
        update(
            float deltaTime)
    {
        if (!owner)
        {
            return;
        }

        sf::Vector2f
            movementInput =
            readMovementInput();

        updateFacingDirection(
            movementInput);

        owner->isMoving =
            movementInput !=
            sf::Vector2f(
                0.f,
                0.f);

        movementInput =
            normalize(
                movementInput);

        smoothedInput_ +=
            (
                movementInput -
                smoothedInput_
                ) *
            turnSpeed_ *
            deltaTime;

        const float
            smoothedLength =
            vectorLength(
                smoothedInput_);

        if (
            smoothedLength >
            INPUT_THRESHOLD)
        {
            applyMovement(
                smoothedInput_,
                deltaTime);
        }
        else
        {
            applyStopping(
                deltaTime);
        }

        updateRollingState(
            deltaTime);
    }

    sf::Vector2f
        MovementComponent::
        readMovementInput() const
    {
        sf::Vector2f
            input(
                0.f,
                0.f);

        // --- ADD THIS SAFETY CHECK ---
        if (!game_)
        {
            // If there's no game pointer (e.g., this is an enemy), 
            // return zero input. AI will handle movement elsewhere.
            return input;
        }

        if (
            sf::Keyboard
            ::isKeyPressed(
                game_->keyUp))
        {
            input.y -=
                1.f;
        }

        if (
            sf::Keyboard
            ::isKeyPressed(
                game_
                ->keyDown))
        {
            input.y +=
                1.f;
        }

        if (
            sf::Keyboard
            ::isKeyPressed(
                game_
                ->keyLeft))
        {
            input.x -=
                1.f;
        }

        if (
            sf::Keyboard
            ::isKeyPressed(
                game_
                ->keyRight))
        {
            input.x +=
                1.f;
        }

        return input;
    }

    void MovementComponent::
        updateFacingDirection(
            const sf::Vector2f&
            direction)
    {
        if (
            direction.x >
            LOOK_THRESHOLD)
        {
            owner
                ->facingRight =
                true;
        }
        else if (
            direction.x <
            -LOOK_THRESHOLD)
        {
            owner
                ->facingRight =
                false;
        }
    }

    void MovementComponent::
        applyMovement(
            const sf::Vector2f&
            direction,
            float deltaTime)
    {
        owner->velocity +=
            direction *
            acceleration_ *
            deltaTime;

        owner->velocity -=
            owner->velocity *
            activeDrag_ *
            deltaTime;
    }

    void MovementComponent::
        applyStopping(
            float deltaTime)
    {
        owner->velocity -=
            owner->velocity *
            stopDrag_ *
            deltaTime;

        const float speed =
            vectorLength(
                owner
                ->velocity);

        if (
            speed <
            STOP_THRESHOLD)
        {
            owner->velocity =
            {
                0.f,
                0.f
            };
        }
    }

    void MovementComponent::
        updateRollingState(
            float deltaTime)
    {
        if (
            owner
            ->actionTimer >
            0.0f)
        {
            owner
                ->actionTimer -=
                deltaTime;

            if (
                owner
                ->isRolling)
            {
                limitSpeed(
                    owner
                    ->overrideSpeedLimit);
            }

            return;
        }

        owner
            ->isRolling =
            false;

        limitSpeed(
            maxSpeed_);
    }

    void MovementComponent::
        limitSpeed(
            float maxSpeed)
    {
        const float
            speed =
            vectorLength(
                owner
                ->velocity);

        if (
            speed <=
            maxSpeed ||
            speed <=
            MIN_VECTOR_LENGTH)
        {
            return;
        }

        owner->velocity =
            (
                owner
                ->velocity /
                speed
                ) *
            maxSpeed;
    }

    float MovementComponent::
        vectorLength(
            const sf::Vector2f&
            vector) const
    {
        return std::sqrt(
            vector.x *
            vector.x +
            vector.y *
            vector.y);
    }

    sf::Vector2f
        MovementComponent::
        normalize(
            const sf::Vector2f&
            vector) const
    {
        const float
            length =
            vectorLength(
                vector);

        if (
            length <=
            MIN_VECTOR_LENGTH)
        {
            return {
                0.f,
                0.f
            };
        }

        return
            vector /
            length;
    }
}