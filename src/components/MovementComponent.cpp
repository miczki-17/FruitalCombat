// --- MovementComponent.cpp ---

#include "MovementComponent.h"
#include "../entities/Entity.h"
#include "PlayerInputComponent.h"
#include "TransformComponent.h"
#include "StatsComponent.h"
#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float STOP_THRESHOLD = 10.0f;
        constexpr float LOOK_THRESHOLD = 0.1f;
        constexpr float INPUT_THRESHOLD = 0.05f;
    }

    MovementComponent::MovementComponent(game::Game* game, float maxSpeed)
        : maxSpeed_(maxSpeed)
    {
        // game_ nie jest juz polem tej klasy. Fizyka nie potrzebuje klawiatury.
    }

    void MovementComponent::setGamePointer(game::Game* game)
    {
        // Sprytny punkt wstrzyknie?cia: gdy PlayingState wywola ta funkcj na graczu,
        // automatycznie dodamy mu komponent wejsciowy obslugujacy klawiature.
        if (game && owner && !owner->getComponent<PlayerInputComponent>())
        {
            owner->addComponent(std::make_unique<PlayerInputComponent>(game));
        }
    }

    void MovementComponent::setDesiredDirection(const sf::Vector2f& dir)
    {
        desiredDirection_ = dir;
    }

    void MovementComponent::update(float deltaTime)
    {
        if (!owner) return;
        auto* transform = owner->getComponent<TransformComponent>();
        if (!transform) return;

        updateFacingDirection(desiredDirection_);

        transform->isMoving = (desiredDirection_ != sf::Vector2f(0.f, 0.f));

        // Wygladzeniue wektora ruchu postaci
        smoothedInput_ += (desiredDirection_ - smoothedInput_) * turnSpeed_ * deltaTime;

        float smoothedLength = smoothedInput_.length();

        if (smoothedLength > INPUT_THRESHOLD)
        {
            applyMovement(smoothedInput_, deltaTime);
        }
        else
        {
            applyStopping(deltaTime);
        }

        updateRollingState(deltaTime);

        desiredDirection_ = { 0.f, 0.f };
    }

    void MovementComponent::updateFacingDirection(const sf::Vector2f& direction)
    {
        auto* transform = owner->getComponent<TransformComponent>();
        if (direction.x > LOOK_THRESHOLD)       transform->facingRight = true;
        else if (direction.x < -LOOK_THRESHOLD) transform->facingRight = false;
    }

    void MovementComponent::applyMovement(const sf::Vector2f& direction, float deltaTime)
    {
        auto* transform = owner->getComponent<TransformComponent>();
        if (!transform) return;

        // 1. Zdob?d? docelow? pr?dko?? (uwzgl?dnia te? np. spowolnienie z trucizny)
        float currentTargetSpeed = maxSpeed_;
        if (auto* stats = owner->getComponent<StatsComponent>())
        {
            currentTargetSpeed = stats->getCurrentSpeed();
        }

        // 2. Dynamiczne przyspieszenie = Pr?dko?? docelowa * Opór (Drag)
        float dynamicAcceleration = currentTargetSpeed * activeDrag_;

        // 3. Aplikuj si?y na wektor
        transform->velocity += direction * dynamicAcceleration * deltaTime;
        transform->velocity -= transform->velocity * activeDrag_ * deltaTime;
    }

    void MovementComponent::applyStopping(float deltaTime)
    {
        auto* transform = owner->getComponent<TransformComponent>();
        transform->velocity -= transform->velocity * stopDrag_ * deltaTime;

        if (transform->velocity.length() < STOP_THRESHOLD)
        {
            transform->velocity = { 0.f, 0.f };
        }
    }

    void MovementComponent::updateRollingState(float deltaTime)
    {
        auto* transform = owner->getComponent<TransformComponent>();
        if (!transform) return;

        if (transform->actionTimer > 0.0f)
        {
            transform->actionTimer -= deltaTime;

            if (transform->isRolling)
            {
                limitSpeed(transform->overrideSpeedLimit);
            }

            return;
        }

        transform->isRolling = false;

        float speedLimit = maxSpeed_;

        if (auto* stats = owner->getComponent<StatsComponent>())
        {
            speedLimit = stats->getCurrentSpeed();
        }

        limitSpeed(speedLimit);
    }

    void MovementComponent::limitSpeed(float maxSpeed)
    {
        auto* transform = owner->getComponent<TransformComponent>();
        float speed = transform->velocity.length(); // SFML 3
        if (speed <= maxSpeed || speed <= 0.001f) return;

        transform->velocity = transform->velocity.normalized() * maxSpeed;
    }

    void MovementComponent::setFriction(const float friction)
    {
        activeDrag_ = friction;
        stopDrag_ = friction * 2.0f; // Stop drag jest zawsze silniejszy, by postac szybciej sie zatrzymywala
    }

    void MovementComponent::setSpeedMultiplier(const float multiplier)
    {
        maxSpeed_ *= multiplier;
    }
}