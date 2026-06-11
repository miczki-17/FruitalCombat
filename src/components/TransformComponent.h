// --- TransformComponent.h ---

#pragma once
#include "Component.h"
#include <SFML/System/Vector2.hpp>

namespace game::components
{
    class TransformComponent final : public Component
    {
    public:
        sf::Vector2f position{ 0.f, 0.f };
        sf::Vector2f velocity{ 0.f, 0.f };

        // Zmienne u¿ywane do sterowania orientacj¹
        bool facingRight = true;
        bool isMoving = false; // Flaga przydatna do animacji
        bool isShooting = false;

        // Flaga akcji specjalnej (np. RindRoll)
        float actionTimer = 0.0f;
        bool isRolling = false;
        float overrideSpeedLimit = 0.0f;

        float zOffset = 0.0f;

        float rotation = 0.0f;
        sf::Vector2f scale = { 1.0f, 1.0f };

        TransformComponent() = default;
        explicit TransformComponent(sf::Vector2f startPos) : position(startPos) {}
    };
}