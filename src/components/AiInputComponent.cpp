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

        constexpr float STATIONARY_DISTANCE = 500.0f;
        constexpr float KAMIKAZE_EXPLODE_RANGE = 45.0f;
    }

    AiInputComponent::AiInputComponent(
        game::entities::Entity* targetPlayer,
        game::genetics::AiBehavior behavior,
        float movementSpeed,
        float agility)
        : targetPlayer_(targetPlayer),
        behavior_(behavior),
        movementSpeed_(movementSpeed),
        agility_(agility)
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

        sf::Vector2f desiredDirection = calculateDesiredDirection(toPlayer, distance);

        if (currentDirection_.x == 0.0f && currentDirection_.y == 0.0f) {
            currentDirection_ = desiredDirection;
        }

        // --- NOWE, AGRESYWNE SKR?CANIE ---
        // Mno?ymy agility_ * n.f, ?eby skala 1-10 by?a brutalnie odczuwalna.
        // std::clamp ucina wynik do 1.0f (czyli max 100% obrotu w jednej klatce), 
        // co chroni fizyk? przed b??dami, gdy wpiszesz w JSON np. 1000.
        float turnSpeed = std::clamp(agility_ * 8.0f * deltaTime, 0.0f, 1.0f);

        currentDirection_.x += (desiredDirection.x - currentDirection_.x) * turnSpeed;
        currentDirection_.y += (desiredDirection.y - currentDirection_.y) * turnSpeed;

        // Przekazanie WYG?ADZONEJ decyzji o kierunku do komponentu ruchu
        if (auto* moveComp = owner->getComponent<MovementComponent>())
        {
            moveComp->setDesiredDirection(game::utils::math::safeNormalize(currentDirection_));
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

        case game::genetics::AiBehavior::Stationary:
            return calculateStationaryMovement(directionToPlayer, distanceToPlayer);

        case game::genetics::AiBehavior::Kamikaze:
            return directionToPlayer; // Szar?uje prosto na cel
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

    sf::Vector2f AiInputComponent::calculateStationaryMovement(
        const sf::Vector2f& directionToPlayer, float distanceToPlayer) const
    {
        // Margines b??du, ?eby Kukurydza nie trz?s?a si? przód/ty?
        float distanceTolerance = 20.0f;

        if (distanceToPlayer > STATIONARY_DISTANCE + distanceTolerance) {
            return directionToPlayer; // Za daleko -> id? do gracza
        }
        else if (distanceToPlayer < STATIONARY_DISTANCE - distanceTolerance) {
            return -directionToPlayer; // Za blisko -> uciekaj
        }

        return { 0.0f, 0.0f }; // W idealnym zasi?gu = zatrzymaj si? ca?kowicie
    }

    void AiInputComponent::tryAttack(float distanceToPlayer)
    {
        auto* abilityComponent = owner->getComponent<AbilityComponent>();
        if (!abilityComponent) return;

        auto* targetPlayer_transform = targetPlayer_->getComponent<TransformComponent>();
        if (!targetPlayer_transform) return;

        // Specjalna logika dla Kamikaze (Czosnek)
        if (behavior_ == game::genetics::AiBehavior::Kamikaze)
        {
            if (distanceToPlayer <= KAMIKAZE_EXPLODE_RANGE) {
                // Kamikaze odpala skill (wybuch) po dotarciu do gracza
                abilityComponent->useSkill(targetPlayer_transform->position);
            }
            return; // Kamikaze nie strzela z normalnej broni, wi?c ko?czymy funkcj?
        }

        // Standardowa logika dla reszty wrogów (Strzelanie z dystansu)
        if (distanceToPlayer <= ATTACK_RANGE)
        {
            abilityComponent->useWeapon(targetPlayer_transform->position);
        }
    }

    void AiInputComponent::tryUseSkill()
    {
        if (auto* abilityComponent = owner->getComponent<AbilityComponent>())
        {
            auto* targetPlayer_transform = targetPlayer_->getComponent<TransformComponent>();
            if (!targetPlayer_transform) return;

            abilityComponent->useSkill(targetPlayer_transform->position);
        }
    }
}