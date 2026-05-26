// --- DNAComponent.cpp ---

#include "DNAComponent.h"
#include "../entities/Entity.h"

#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float SURVIVAL_SCORE_PER_SEC =
            10.0f;

        constexpr float PROXIMITY_RANGE =
            300.0f;

        constexpr float PROXIMITY_MULTIPLIER =
            0.1f;
    }

    DNAComponent::DNAComponent(
        const game::genetics::DNA& dna,
        game::entities::Entity* player)
        : dna_(dna),
        targetPlayer_(player)
    {
    }

    void DNAComponent::update(
        float deltaTime)
    {
        if (!owner || !targetPlayer_ || owner->isDead)
        {
            return;
        }

        addSurvivalFitness(deltaTime);
        addProximityFitness(deltaTime);
    }

    void DNAComponent::addSurvivalFitness(
        float deltaTime)
    {
        dna_.fitnessScore +=
            SURVIVAL_SCORE_PER_SEC *
            deltaTime;
    }

    void DNAComponent::addProximityFitness(
        float deltaTime)
    {
        const float distance =
            calculateDistanceToPlayer();

        if (distance >= PROXIMITY_RANGE)
        {
            return;
        }

        dna_.fitnessScore +=
            (
                PROXIMITY_RANGE -
                distance
                ) *
            deltaTime *
            PROXIMITY_MULTIPLIER;
    }

    float DNAComponent::calculateDistanceToPlayer()
        const
    {
        const sf::Vector2f diff =
            targetPlayer_->position -
            owner->position;

        return std::sqrt(
            diff.x * diff.x +
            diff.y * diff.y);
    }

    const game::genetics::DNA& DNAComponent::getDNA() 
        const
    {
        return dna_;
    }
}