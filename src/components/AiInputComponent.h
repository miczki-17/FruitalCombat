// ==========================================
// components/AiInputComponent.h
// ==========================================
#pragma once
#include "Component.h"
#include "../genetics/DNA.h"
#include <random>

namespace game::entities { class Entity; }

namespace game::components
{
    class AiInputComponent final : public Component
    {
    public:
        AiInputComponent(
            game::entities::Entity* targetPlayer,
            game::genetics::AiBehavior behavior,
            float movementSpeed);

        void update(float deltaTime) override;

    private:
        game::entities::Entity* targetPlayer_;
        game::genetics::AiBehavior behavior_;
        float movementSpeed_;

        float decisionTimer_ = 0.0f;
        float strafeDirection_ = 1.0f;

        std::mt19937 randomEngine_{ std::random_device{}() };
        std::uniform_real_distribution<float> decisionTimeDistribution_{ 1.0f, 3.0f };
        std::uniform_int_distribution<int> directionDistribution_{ 0, 1 };

        void updateDecision(float deltaTime);
        sf::Vector2f calculateDesiredDirection(const sf::Vector2f& directionToPlayer, float distanceToPlayer) const;
        sf::Vector2f calculateSniperMovement(const sf::Vector2f& directionToPlayer, float distanceToPlayer) const;
        sf::Vector2f calculateSkirmisherMovement(const sf::Vector2f& directionToPlayer, float distanceToPlayer) const;

        //void applyMovement(const sf::Vector2f& direction, float deltaTime);
        //void updateFacingDirection(const sf::Vector2f& directionToPlayer);
        void tryAttack(float distanceToPlayer);
        void tryUseSkill();
    };
}