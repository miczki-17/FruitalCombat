// --- AiInputComponent.cpp ---

#include "AiInputComponent.h"
#include "AbilityComponent.h"
#include "MovementComponent.h"
#include "TransformComponent.h"
#include "../entities/Entity.h"
#include "../utils/MathUtils.h"
#include <cmath>

namespace game::components
{
    namespace
    {
        constexpr float LOOK_THRESHOLD = 0.1f;
        constexpr float ATTACK_RANGE = 600.0f;
        constexpr float SNIPER_MIN_DISTANCE = 300.0f;
        constexpr float SNIPER_MAX_DISTANCE = 450.0f;
        constexpr float SKIRMISHER_DISTANCE = 200.0f;
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

    void AiInputComponent::update(float deltaTime)
    {
        if (!owner || !targetPlayer_ || targetPlayer_->isDead()) return;

        auto* targetPlayer_transform = targetPlayer_->getComponent<TransformComponent>();
        if (!targetPlayer_transform) return;

        auto* owner_transform = owner->getComponent<TransformComponent>();
        if (!owner_transform) return;

        sf::Vector2f toPlayer = targetPlayer_transform->position - owner_transform->position;
        const float distance = toPlayer.length(); // SFML 3

        toPlayer = game::utils::math::safeNormalize(toPlayer);

        updateDecision(deltaTime);

        sf::Vector2f movementDirection = calculateDesiredDirection(toPlayer, distance);

        // Przekazanie decyzji o kierunku do komponentu ruchu
        if (auto* moveComp = owner->getComponent<MovementComponent>())
        {
            moveComp->setDesiredDirection(game::utils::math::safeNormalize(movementDirection));
        }

        tryAttack(distance);
    }

    void AiInputComponent::updateDecision(float deltaTime)
    {
        decisionTimer_ -= deltaTime;
        if (decisionTimer_ > 0.0f) return;

        decisionTimer_ = decisionTimeDistribution_(randomEngine_);
        strafeDirection_ = (directionDistribution_(randomEngine_) == 0) ? -1.0f : 1.0f;

        if (behavior_ == game::genetics::AiBehavior::Skirmisher)
        {
            tryUseSkill();
        }
    }

    sf::Vector2f AiInputComponent::calculateDesiredDirection(
        const sf::Vector2f& directionToPlayer, float distanceToPlayer) const
    {
        switch (behavior_)
        {
        case game::genetics::AiBehavior::Charger:
            return directionToPlayer;

        case game::genetics::AiBehavior::Sniper:
            return calculateSniperMovement(directionToPlayer, distanceToPlayer);

        case game::genetics::AiBehavior::Skirmisher:
            return calculateSkirmisherMovement(directionToPlayer, distanceToPlayer);
        }
        return { 0.f, 0.f };
    }

    sf::Vector2f AiInputComponent::calculateSniperMovement(
        const sf::Vector2f& directionToPlayer, float distanceToPlayer) const
    {
        if (distanceToPlayer > SNIPER_MAX_DISTANCE) return directionToPlayer;
        if (distanceToPlayer < SNIPER_MIN_DISTANCE) return -directionToPlayer;

        sf::Vector2f orbitDirection(-directionToPlayer.y * strafeDirection_, directionToPlayer.x * strafeDirection_);
        return orbitDirection * 0.8f + directionToPlayer * 0.2f;
    }

    sf::Vector2f AiInputComponent::calculateSkirmisherMovement(
        const sf::Vector2f& directionToPlayer, float distanceToPlayer) const
    {
        if (distanceToPlayer > SKIRMISHER_DISTANCE) return directionToPlayer;
        sf::Vector2f evadeDirection(-directionToPlayer.y * strafeDirection_, directionToPlayer.x * strafeDirection_);
        return evadeDirection - directionToPlayer * 0.5f;
    }

    //void AiInputComponent::applyMovement(const sf::Vector2f& direction, float deltaTime)
    //{
    //    // 1. DROGA ECS: Je?li wróg ma dedykowany komponent ruchu, zlecamy mu zadanie
    //    if (auto* moveComp = owner->getComponent<MovementComponent>())
    //    {
    //        moveComp->setDesiredDirection(direction);
    //        return;
    //    }

    //    auto* owner_transform = owner->getComponent<TransformComponent>(); if (!owner_transform) return;

    //    // 2. DROGA LEGACY: Je?li mutant nie ma jeszcze komponentu ruchu, liczymy fizyk? bezpo?rednio (Etap 2 fallback)
    //    owner_transform->isMoving = (direction != sf::Vector2f(0.f, 0.f));
    //    owner_transform->velocity += direction * acceleration_ * deltaTime;
    //    owner_transform->velocity -= owner_transform->velocity * drag_ * deltaTime;

    //    float speed = owner_transform->velocity.length(); // SFML 3
    //    if (speed > movementSpeed_ && !owner_transform->isRolling && owner_transform->actionTimer <= 0.0f)
    //    {
    //        owner_transform->velocity = owner_transform->velocity.normalized() * movementSpeed_; // SFML 3
    //    }
    //}

    //void AiInputComponent::updateFacingDirection(const sf::Vector2f& directionToPlayer)
    //{
    //    auto* owner_transform = owner->getComponent<TransformComponent>(); if (!owner_transform) return;

    //    if (directionToPlayer.x > LOOK_THRESHOLD)       owner_transform->facingRight = true;
    //    else if (directionToPlayer.x < -LOOK_THRESHOLD) owner_transform->facingRight = false;
    //}

    void AiInputComponent::tryAttack(float distanceToPlayer)
    {
        if (distanceToPlayer > ATTACK_RANGE) return;
        if (auto* abilityComponent = owner->getComponent<AbilityComponent>())
        {
            auto* targetPlayer_transform = targetPlayer_->getComponent<TransformComponent>(); if (!targetPlayer_transform) return;
            abilityComponent->useWeapon(targetPlayer_transform->position);
        }
    }

    void AiInputComponent::tryUseSkill()
    {
        if (auto* abilityComponent = owner->getComponent<AbilityComponent>())
        {
            auto* targetPlayer_transform = targetPlayer_->getComponent<TransformComponent>(); if (!targetPlayer_transform) return;
            abilityComponent->useSkill(targetPlayer_transform->position);
        }
    }
}