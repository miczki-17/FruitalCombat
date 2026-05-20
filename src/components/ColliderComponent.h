#pragma once
#include "Component.h"
#include "../entities/Entity.h"
#include <SFML/Graphics.hpp>
#include <cmath>

namespace game::components
{
    class ColliderComponent : public Component
    {
    private:
        const sf::Image& collisionMask;
        float mapScale;
        float radius;

    public:
        ColliderComponent(const sf::Image& mask, float scale, float colRadius = 25.0f)
            : collisionMask(mask), mapScale(scale), radius(colRadius) {
        }

        void update(float dt) override
        {
            sf::Vector2f nextPos = owner->position + owner->velocity * dt;
            sf::Vector2u maskSize = collisionMask.getSize();

            sf::Vector2f pushVector(0.f, 0.f);
            int hitCount = 0;
            const int numProbes = 24;

            // 1 faza: Wykrywanie sciany
            for (int i = 0; i < numProbes; ++i)
            {
                float angle = (i * 2.0f * 3.14159265f) / numProbes;
                sf::Vector2f offset(std::cos(angle) * radius, std::sin(angle) * radius);
                sf::Vector2f probePoint = nextPos + offset;

                int px = static_cast<int>(probePoint.x / mapScale);
                int py = static_cast<int>(probePoint.y / mapScale);

                bool isBlackPixel = (px < 0 || px >= static_cast<int>(maskSize.x) ||
                    py < 0 || py >= static_cast<int>(maskSize.y)) ||
                    (collisionMask.getPixel(sf::Vector2u(px, py)) == sf::Color::Black);

                if (isBlackPixel)
                {
                    hitCount++;
                    pushVector -= offset;
                }
            }

            if (hitCount > 0)
            {
                float pushLength = std::sqrt(pushVector.x * pushVector.x + pushVector.y * pushVector.y);
                if (pushLength > 0.001f)
                {
                    sf::Vector2f collisionNormal = pushVector / pushLength;
                    float dotProduct = owner->velocity.x * collisionNormal.x + owner->velocity.y * collisionNormal.y;

                    if (dotProduct < 0.f)
                    {
                        owner->velocity -= collisionNormal * dotProduct;
                        owner->velocity -= owner->velocity * 0.03f; // tarcie o sciany
                    }
                    nextPos = owner->position + owner->velocity * dt;
                }
            }

            // 2 faza: Wypychanie z wnetrza sciany (3 iteracje)
            for (int step = 0; step < 3; ++step)
            {
                sf::Vector2f currentPush(0.f, 0.f);
                int currentHits = 0;

                for (int i = 0; i < numProbes; ++i)
                {
                    float angle = (i * 2.0f * 3.14159265f) / numProbes;
                    sf::Vector2f offset(std::cos(angle) * radius, std::sin(angle) * radius);
                    sf::Vector2f probePoint = nextPos + offset;

                    int px = static_cast<int>(probePoint.x / mapScale);
                    int py = static_cast<int>(probePoint.y / mapScale);

                    if (px < 0 || px >= static_cast<int>(maskSize.x) || py < 0 || py >= static_cast<int>(maskSize.y) ||
                        collisionMask.getPixel(sf::Vector2u(px, py)) == sf::Color::Black)
                    {
                        currentPush -= offset;
                        currentHits++;
                    }
                }

                if (currentHits == 0) break;

                float length = std::sqrt(currentPush.x * currentPush.x + currentPush.y * currentPush.y);
                if (length > 0.001f) {
                    nextPos += (currentPush / length) * 0.6f;
                }
                else {
                    nextPos.y += 0.5f;
                }
            }

            owner->position = nextPos;
        }
    };
}