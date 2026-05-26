// --- MovementComponent.h ---  

#pragma once
#include "Component.h"

namespace game
{
    class Game;
}

namespace game::components
{
    class MovementComponent final : public Component
    {
    public:
        MovementComponent(game::Game* game, float maxSpeed);

        void update(float deltaTime) override;

        // Mostek kompatybilno?ci dla PlayingState: automatycznie dopina PlayerInputComponent do gracza
        void setGamePointer(game::Game* game);

        // Nowe, czyste API systemu gier: pozwala zewn?trznym komponentom sterowa? t? encj?
        void setDesiredDirection(const sf::Vector2f& dir);

    private:
        float maxSpeed_;
        float acceleration_ = 2500.0f;
        float activeDrag_ = 4.0f;
        float stopDrag_ = 8.0f;
        float turnSpeed_ = 15.0f;

        sf::Vector2f desiredDirection_{ 0.f, 0.f };
        sf::Vector2f smoothedInput_{ 0.f, 0.f };

        void updateFacingDirection(const sf::Vector2f& direction);
        void applyMovement(const sf::Vector2f& direction, float deltaTime);
        void applyStopping(float deltaTime);
        void updateRollingState(float deltaTime);
        void limitSpeed(float maxSpeed);
    };
}