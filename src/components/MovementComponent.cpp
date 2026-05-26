// --- MovementComponent.cpp ---

#include "MovementComponent.h"
#include "../entities/Entity.h"
#include "PlayerInputComponent.h"
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
        // game_ nie jest ju? polem tej klasy. Fizyka nie potrzebuje klawiatury!
    }

    void MovementComponent::setGamePointer(game::Game* game)
    {
        // Sprytny punkt wstrzykni?cia: gdy PlayingState wywo?a t? funkcj? na graczu,
        // automatycznie dodamy mu komponent wej?ciowy obs?uguj?cy klawiatur?.
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

        updateFacingDirection(desiredDirection_);

        owner->isMoving = (desiredDirection_ != sf::Vector2f(0.f, 0.f));

        // P?ynne wyg?adzanie wektora ruchu (wyg?adza zwroty postaci)
        smoothedInput_ += (desiredDirection_ - smoothedInput_) * turnSpeed_ * deltaTime;

        // NOWO?? SFML 3: Bezpo?rednie pobranie d?ugo?ci wektora
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

        // Bardzo wa?ne: Czy?cimy intencj? na koniec klatki. 
        // W nast?pnej klatce Input (gracz lub AI) musi znowu ustawi? kierunek.
        desiredDirection_ = { 0.f, 0.f };
    }

    void MovementComponent::updateFacingDirection(const sf::Vector2f& direction)
    {
        if (direction.x > LOOK_THRESHOLD)       owner->facingRight = true;
        else if (direction.x < -LOOK_THRESHOLD) owner->facingRight = false;
    }

    void MovementComponent::applyMovement(const sf::Vector2f& direction, float deltaTime)
    {
        owner->velocity += direction * acceleration_ * deltaTime;
        owner->velocity -= owner->velocity * activeDrag_ * deltaTime;
    }

    void MovementComponent::applyStopping(float deltaTime)
    {
        owner->velocity -= owner->velocity * stopDrag_ * deltaTime;

        // NOWO?? SFML 3: .length() zamiast r?cznego sqrt
        if (owner->velocity.length() < STOP_THRESHOLD)
        {
            owner->velocity = { 0.f, 0.f };
        }
    }

    void MovementComponent::updateRollingState(float deltaTime)
    {
        if (owner->actionTimer > 0.0f)
        {
            owner->actionTimer -= deltaTime;
            if (owner->isRolling)
            {
                limitSpeed(owner->overrideSpeedLimit);
            }
            return;
        }

        owner->isRolling = false;
        limitSpeed(maxSpeed_);
    }

    void MovementComponent::limitSpeed(float maxSpeed)
    {
        float speed = owner->velocity.length(); // SFML 3
        if (speed <= maxSpeed || speed <= 0.001f) return;

        // NOWO?? SFML 3: .normalized() produkuje czysty wektor jednostkowy bez modyfikacji orygina?u
        owner->velocity = owner->velocity.normalized() * maxSpeed;
    }
}