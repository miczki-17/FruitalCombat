// --- ColliderComponent.cpp --- 

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "../entities/Entity.h"

#include <cmath>
#include <numbers>

namespace game::components
{
    namespace
    {
        constexpr int NUM_PROBES =
            24;

        constexpr int PUSH_OUT_ITERATIONS =
            3;

        constexpr float WALL_FRICTION =
            0.03f;

        constexpr float PUSH_OUT_STEP =
            0.6f;

        constexpr float FALLBACK_PUSH =
            0.5f;

        constexpr float MIN_VECTOR_LENGTH =
            0.001f;
    }

    ColliderComponent::ColliderComponent(
        const sf::Image& collisionMask,
        float mapScale,
        float collisionRadius)
        : collisionMask_(collisionMask),
        mapScale_(mapScale),
        collisionRadius_(collisionRadius)
    {
    }

    void ColliderComponent::update(
        float deltaTime)
    {
        if (!owner)
        {
            return;
        }
        auto* transform = owner->getComponent<TransformComponent>();
        if (!transform) return;

        sf::Vector2f nextPosition =
            transform->position +
            transform->velocity *
            deltaTime;

        resolveWallCollision(
            nextPosition,
            deltaTime);

        pushOutsideWalls(
            nextPosition);

        transform->position =
            nextPosition;
    }

    bool ColliderComponent::isWallPixel(
        const sf::Vector2f& worldPosition) const
    {
        const sf::Vector2u maskSize =
            collisionMask_.getSize();

        const int pixelX =
            static_cast<int>(
                worldPosition.x /
                mapScale_);

        const int pixelY =
            static_cast<int>(
                worldPosition.y /
                mapScale_);

        const bool outOfBounds =
            pixelX < 0 ||
            pixelY < 0 ||
            pixelX >=
            static_cast<int>(
                maskSize.x) ||
            pixelY >=
            static_cast<int>(
                maskSize.y);

        if (outOfBounds)
        {
            return true;
        }

        return
            collisionMask_.getPixel(
                sf::Vector2u(
                    pixelX,
                    pixelY)) ==
            sf::Color::Black;
    }

    sf::Vector2f
        ColliderComponent::calculatePushVector(
            const sf::Vector2f& position,
            int& hitCount) const
    {
        sf::Vector2f pushVector(
            0.f,
            0.f);

        hitCount = 0;

        for (
            int probeIndex = 0;
            probeIndex < NUM_PROBES;
            ++probeIndex)
        {
            const float angle =
                (
                    static_cast<float>(
                        probeIndex) *
                    2.0f *
                    std::numbers::pi_v<float>
                    ) /
                NUM_PROBES;

            const sf::Vector2f offset(
                std::cos(angle) *
                collisionRadius_,

                std::sin(angle) *
                collisionRadius_);

            const sf::Vector2f probePoint =
                position +
                offset;

            if (!isWallPixel(
                probePoint))
            {
                continue;
            }

            ++hitCount;

            pushVector -=
                offset;
        }

        return pushVector;
    }

    void ColliderComponent::resolveWallCollision(
        sf::Vector2f& nextPosition,
        float deltaTime)
    {
        auto* transform = owner->getComponent<TransformComponent>();
        int hitCount = 0;

        const sf::Vector2f pushVector =
            calculatePushVector(
                nextPosition,
                hitCount);

        if (hitCount <= 0)
        {
            return;
        }

        const float pushLength =
            vectorLength(
                pushVector);

        if (
            pushLength <=
            MIN_VECTOR_LENGTH)
        {
            return;
        }

        const sf::Vector2f normal =
            pushVector /
            pushLength;

        const float velocityDot =
            transform->velocity.x *
            normal.x +
            transform->velocity.y *
            normal.y;

        if (velocityDot < 0.f)
        {
            transform->velocity -=
                normal *
                velocityDot;

            transform->velocity -=
                transform->velocity *
                WALL_FRICTION;
        }

        nextPosition =
            transform->position +
            transform->velocity *
            deltaTime;
    }

    void ColliderComponent::pushOutsideWalls(
        sf::Vector2f& nextPosition)
    {
        for (
            int iteration = 0;
            iteration <
            PUSH_OUT_ITERATIONS;
            ++iteration)
        {
            int hitCount = 0;

            const sf::Vector2f pushVector =
                calculatePushVector(
                    nextPosition,
                    hitCount);

            if (hitCount == 0)
            {
                return;
            }

            const float length =
                vectorLength(
                    pushVector);

            if (
                length >
                MIN_VECTOR_LENGTH)
            {
                nextPosition +=
                    (
                        pushVector /
                        length
                        ) *
                    PUSH_OUT_STEP;

                continue;
            }

            nextPosition.y +=
                FALLBACK_PUSH;
        }
    }

    float ColliderComponent::vectorLength(
        const sf::Vector2f& vector) const
    {
        return std::sqrt(
            vector.x *
            vector.x +
            vector.y *
            vector.y);
    }

    sf::Vector2f
        ColliderComponent::normalize(
            const sf::Vector2f& vector) const
    {
        const float length =
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

        return vector /
            length;
    }
}