// --- AcidSquirtAbility.h ---

#pragma once

#include "Ability.h"
#include <string>
#include <vector>

namespace game { struct ArenaContext; }
namespace game::entities { class Entity; }

namespace game::components
{
    class AcidSquirtAbility final : public Ability
    {
    public:
        AcidSquirtAbility(
            game::ArenaContext* context,
            game::entities::Entity* owner,
            const std::string& textureKey,
            const std::string& splashKeyBase,
            float projectileScale = 1.0f,
            bool isFriendly = true,
            std::string sourceName = "Undefined Biomass");

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        game::ArenaContext* context_;
        game::entities::Entity* owner_;

        std::string textureKey_;
        std::string splashKeyBase_;
        float projectileScale_;
        bool isFriendly_;

        float baseCooldown_ = 0.6f;
        float cooldownTimer_ = 0.0f;

        // mana cost
        float manaCost_ = 25.0f;

        void spawnProjectile(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition);

        float getAttackSpeedModifier() const;
        int getBonusProjectileCount() const;

        bool isOnCooldown() const;
        void resetCooldown(float attackSpeedModifier);
    };
}