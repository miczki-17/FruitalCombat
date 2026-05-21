#pragma once
#include "Component.h"
#include "../genetics/DNA.h"
#include "../entities/Entity.h"

namespace game::components
{
    class DNAComponent : public Component
    {
    public:
        game::genetics::DNA dna;
        game::entities::Entity* targetPlayer = nullptr;

        DNAComponent(const game::genetics::DNA& initialDna, game::entities::Entity* player)
            : dna(initialDna), targetPlayer(player) {
        }

        void update(float dt) override
        {
            if (!owner || !targetPlayer || owner->isDead) return;

            // 1. Base Fitness: Surviving gives points
            dna.fitnessScore += dt * 10.0f;

            // 2. Proximity Fitness: Being close to the player is rewarded
            sf::Vector2f diff = targetPlayer->position - owner->position;
            float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

            if (distance < 300.0f) {
                // Closer = more points (encourages aggressive AI evolution)
                dna.fitnessScore += (300.0f - distance) * dt * 0.1f;
            }
        }
    };
}