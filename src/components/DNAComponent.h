// --- DNAComponent.h ---

#pragma once

#include "Component.h"
#include "../genetics/DNA.h"

namespace game::components
{
    class DNAComponent final : public Component
    {
    public:
        DNAComponent(
            const game::genetics::DNA& dna,
            game::entities::Entity* player);

        void update(float deltaTime) override;

        const game::genetics::DNA& getDNA() const;

    private:
        game::genetics::DNA dna_;
        game::entities::Entity* targetPlayer_;

        void addSurvivalFitness(
            float deltaTime);

        void addProximityFitness(
            float deltaTime);

        float calculateDistanceToPlayer() const;
    };
}