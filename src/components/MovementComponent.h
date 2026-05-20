#pragma once
#include "Component.h"
#include "../core/Game.h"
#include "../entities/Entity.h"
#include <cmath>

namespace game::components
{
    class MovementComponent : public Component
    {
    private:
        game::Game* game;

        float maxSpeed;
        float acceleration = 2500.0f;
        float activeDrag = 4.0f;
        float stopDrag = 8.0f;
        float turnSpeed = 15.0f;

        sf::Vector2f smoothedInput;

    public:
        MovementComponent(game::Game* gameRef, float speed)
            : game(gameRef), maxSpeed(speed) {
        }

        void update(float dt) override
        {
            sf::Vector2f targetDir(0.f, 0.f);

            if (sf::Keyboard::isKeyPressed(game->keyUp))    targetDir.y -= 1.f;
            if (sf::Keyboard::isKeyPressed(game->keyDown))  targetDir.y += 1.f;
            if (sf::Keyboard::isKeyPressed(game->keyLeft))  targetDir.x -= 1.f;
            if (sf::Keyboard::isKeyPressed(game->keyRight)) targetDir.x += 1.f;

            if (targetDir.x > 0.1f)       owner->facingRight = true;
            else if (targetDir.x < -0.1f) owner->facingRight = false;

            owner->isMoving = (targetDir.x != 0.f || targetDir.y != 0.f);

            if (owner->isMoving)
            {
                float length = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
                targetDir.x /= length;
                targetDir.y /= length;
            }

            smoothedInput += (targetDir - smoothedInput) * turnSpeed * dt;

            float smoothedLength = std::sqrt(smoothedInput.x * smoothedInput.x + smoothedInput.y * smoothedInput.y);
            if (smoothedLength > 0.05f)
            {
                owner->velocity += smoothedInput * acceleration * dt;
                owner->velocity -= owner->velocity * activeDrag * dt;
            }
            else
            {
                owner->velocity -= owner->velocity * stopDrag * dt;
                if (std::sqrt(owner->velocity.x * owner->velocity.x + owner->velocity.y * owner->velocity.y) < 10.0f)
                    owner->velocity = { 0.f, 0.f };
            }

            if (owner->actionTimer > 0.0f)
            {
                owner->actionTimer -= dt;
                if (owner->isRolling) limitSpeed(owner->overrideSpeedLimit);
            }
            else
            {
                owner->isRolling = false;
                limitSpeed(maxSpeed);
            }
        }

    private:
        void limitSpeed(float limit)
        {
            float currentSpeed = std::sqrt(owner->velocity.x * owner->velocity.x + owner->velocity.y * owner->velocity.y);
            if (currentSpeed > limit && currentSpeed > 0.001f)
            {
                owner->velocity.x = (owner->velocity.x / currentSpeed) * limit;
                owner->velocity.y = (owner->velocity.y / currentSpeed) * limit;
            }
        }
    };
}