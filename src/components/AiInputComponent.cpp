#include "AiInputComponent.h"

#include "AbilityComponent.h"
#include "../entities/Entity.h"

#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float MIN_VECTOR_LENGTH =
            0.001f;

        constexpr float LOOK_THRESHOLD =
            0.1f;

        constexpr float ATTACK_RANGE =
            600.0f;

        constexpr float SNIPER_MIN_DISTANCE =
            300.0f;

        constexpr float SNIPER_MAX_DISTANCE =
            450.0f;

        constexpr float SKIRMISHER_DISTANCE =
            200.0f;
    }

    AiInputComponent::AiInputComponent(
        game::entities::Entity* targetPlayer,
        game::genetics::AiBehavior behavior,
        float movementSpeed)
        : targetPlayer_(targetPlayer),
        behavior_(behavior),
        movementSpeed_(movementSpeed)
    {
    }

    void AiInputComponent::update(
        float deltaTime)
    {
        if (
            !owner ||
            !targetPlayer_ ||
            targetPlayer_->isDead)
        {
            return;
        }

        sf::Vector2f toPlayer =
            targetPlayer_->position -
            owner->position;

        const float distance =
            vectorLength(
                toPlayer);

        toPlayer =
            normalize(
                toPlayer);

        updateDecision(
            deltaTime);

        const sf::Vector2f movementDirection =
            calculateDesiredDirection(
                toPlayer,
                distance);

        applyMovement(
            normalize(
                movementDirection),
            deltaTime);

        updateFacingDirection(
            toPlayer);

        tryAttack(
            distance);
    }

    void AiInputComponent::updateDecision(
        float deltaTime)
    {
        decisionTimer_ -=
            deltaTime;

        if (
            decisionTimer_ >
            0.0f)
        {
            return;
        }

        decisionTimer_ =
            decisionTimeDistribution_(
                randomEngine_);

        strafeDirection_ =
            directionDistribution_(
                randomEngine_)
            == 0
            ? -1.0f
            : 1.0f;

        if (
            behavior_ ==
            game::genetics::AiBehavior::Skirmisher)
        {
            tryUseSkill();
        }
    }

    sf::Vector2f
        AiInputComponent::calculateDesiredDirection(
            const sf::Vector2f& directionToPlayer,
            float distanceToPlayer) const
    {
        switch (behavior_)
        {
        case game::genetics::AiBehavior::Charger:
            return directionToPlayer;

        case game::genetics::AiBehavior::Sniper:
            return calculateSniperMovement(
                directionToPlayer,
                distanceToPlayer);

        case game::genetics::AiBehavior::Skirmisher:
            return calculateSkirmisherMovement(
                directionToPlayer,
                distanceToPlayer);
        }

        return { 0.f, 0.f };
    }

    sf::Vector2f
        AiInputComponent::calculateSniperMovement(
            const sf::Vector2f& directionToPlayer,
            float distanceToPlayer) const
    {
        if (
            distanceToPlayer >
            SNIPER_MAX_DISTANCE)
        {
            return directionToPlayer;
        }

        if (
            distanceToPlayer <
            SNIPER_MIN_DISTANCE)
        {
            return -directionToPlayer;
        }

        sf::Vector2f orbitDirection(
            -directionToPlayer.y *
            strafeDirection_,

            directionToPlayer.x *
            strafeDirection_);

        return
            orbitDirection *
            0.8f +
            directionToPlayer *
            0.2f;
    }

    sf::Vector2f
        AiInputComponent::calculateSkirmisherMovement(
            const sf::Vector2f& directionToPlayer,
            float distanceToPlayer) const
    {
        if (
            distanceToPlayer >
            SKIRMISHER_DISTANCE)
        {
            return directionToPlayer;
        }

        sf::Vector2f evadeDirection(
            -directionToPlayer.y *
            strafeDirection_,

            directionToPlayer.x *
            strafeDirection_);

        return
            evadeDirection -
            directionToPlayer *
            0.5f;
    }

    void AiInputComponent::applyMovement(
        const sf::Vector2f& direction,
        float deltaTime)
    {
        owner->isMoving =
            direction !=
            sf::Vector2f(
                0.f,
                0.f);

        owner->velocity +=
            direction *
            acceleration_ *
            deltaTime;

        owner->velocity -=
            owner->velocity *
            drag_ *
            deltaTime;

        limitVelocity();
    }

    void AiInputComponent::updateFacingDirection(
        const sf::Vector2f& directionToPlayer)
    {
        if (
            directionToPlayer.x >
            LOOK_THRESHOLD)
        {
            owner->facingRight =
                true;
        }
        else if (
            directionToPlayer.x <
            -LOOK_THRESHOLD)
        {
            owner->facingRight =
                false;
        }
    }

    void AiInputComponent::tryAttack(
        float distanceToPlayer)
    {
        if (
            distanceToPlayer >
            ATTACK_RANGE)
        {
            return;
        }

        auto* abilityComponent =
            owner->getComponent<AbilityComponent>();

        if (!abilityComponent)
        {
            return;
        }

        abilityComponent->useWeapon(
            targetPlayer_->position);
    }

    void AiInputComponent::tryUseSkill()
    {
        auto* abilityComponent =
            owner->getComponent<AbilityComponent>();

        if (!abilityComponent)
        {
            return;
        }

        abilityComponent->useSkill(
            targetPlayer_->position);
    }

    void AiInputComponent::limitVelocity()
    {
        const float speed =
            vectorLength(
                owner->velocity);

        if (
            speed <=
            movementSpeed_ ||
            owner->isRolling ||
            owner->actionTimer >
            0.0f)
        {
            return;
        }

        owner->velocity =
            (
                owner->velocity /
                speed
                ) *
            movementSpeed_;
    }

    float AiInputComponent::vectorLength(
        const sf::Vector2f& vector) const
    {
        return std::sqrt(
            vector.x * vector.x +
            vector.y * vector.y);
    }

    sf::Vector2f
        AiInputComponent::normalize(
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

        return vector / length;
    }
}