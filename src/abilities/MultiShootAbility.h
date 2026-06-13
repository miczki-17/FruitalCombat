#pragma once
#include "Ability.h"
#include <string>

namespace game { struct ArenaContext; }
namespace game::entities { class Entity; }

namespace game::components
{
    class MultiShootAbility : public Ability
    {
    public:
        MultiShootAbility(
            game::ArenaContext* context,
            game::entities::Entity* owner,
            const std::string& textureKey,
            int projectileCount = 3,
            float burstDelay = 0.1f,
            float projectileScale = 1.0f,
            float cooldown = 1.0f,
            float damage = 5.0f,
            std::string sourceName = "Undefined Biomass"
        );

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        sf::Vector2f calculateBaseDirection(const sf::Vector2f& origin, const sf::Vector2f& targetPosition) const;
        void spawnProjectile(const sf::Vector2f& origin, const sf::Vector2f& direction, const sf::Vector2f& ownerVelocity);

        bool isOnCooldown() const;
        void resetCooldown();

    private:
        game::ArenaContext* context_;
        game::entities::Entity* owner_;

        std::string textureKey_;
        int projectileCount_;
        float burstDelay_;
        float projectileScale_;
        float cooldown_;
        float damage_;

        float cooldownTimer_ = 0.0f;
        float manaCost_ = 10.0f;

		// Zmienne do serii pocisków
        bool isBursting_ = false;
        int projectilesFired_ = 0;
        float burstTimer_ = 0.0f;
        sf::Vector2f burstDirection_;
        sf::Vector2f burstOwnerVelocity_;
    };
}