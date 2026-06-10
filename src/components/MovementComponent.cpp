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

        // --- POPRAWKA: Rozró?nienie gracza od NPC ---
        bool isPlayer = (owner->getComponent<PlayerInputComponent>() != nullptr);

        if (isPlayer)
        {
            // Gracz zachowuje p?ynne wyg?adzanie sterowania klawiatur?
            smoothedInput_ += (desiredDirection_ - smoothedInput_) * turnSpeed_ * deltaTime;
        }
        else
        {
            // NPC reaguje NATYCHMIAST na swoje AI, eliminuj?c podwójne wyg?adzanie
            smoothedInput_ = desiredDirection_;
        }

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

        float currentTargetSpeed = maxSpeed_;
        if (auto* stats = owner->getComponent<StatsComponent>())
        {
            currentTargetSpeed = stats->getCurrentSpeed();
        }

        // --- Izolacja tarcia mapy ---
        bool isPlayer = (owner->getComponent<PlayerInputComponent>() != nullptr);
        float drag = isPlayer ? activeDrag_ : 3.5f; // NPC ma sta?y grip, gracz s?ucha mapy

        // --- Gwa?towne skr?ty NPC ---
        // Je?li to przeciwnik, porusza si? i próbuje zmieni? kierunek pod du?ym k?tem:
        if (!isPlayer && transform->velocity.length() > 10.0f)
        {
            sf::Vector2f normVel = transform->velocity.normalized();
            // Iloczyn skalarny (dot product) mówi nam, jak bardzo po??dany kierunek ró?ni si? od obecnego
            float cosAngle = normVel.x * direction.x + normVel.y * direction.y;

            // Je?li k?t skr?tu jest ostry lub potwór zawraca (cosAngle < 0.75)
            if (cosAngle < 0.75f)
            {
                // kasujemy p?d boczny
                transform->velocity = direction * transform->velocity.length() * 0.5f;
            }
        }

        // Standardowe zaaplikowanie si?
        float dynamicAcceleration = currentTargetSpeed * drag;
        transform->velocity += direction * dynamicAcceleration * deltaTime;
        transform->velocity -= transform->velocity * drag * deltaTime;
    }

    void MovementComponent::applyStopping(float deltaTime)
    {
        auto* transform = owner->getComponent<TransformComponent>();
        if (!transform) return;

        bool isPlayer = (owner->getComponent<PlayerInputComponent>() != nullptr);
        float drag = isPlayer ? stopDrag_ : 12.0f; // NPC staje jak wryty, gracz ?lizga si? zale?nie od mapy

        transform->velocity -= transform->velocity * drag * deltaTime;

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