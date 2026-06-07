#pragma once

#include "Ability.h"
#include "../core/ArenaContext.h"
#include <string>

namespace game::entities { class Entity; }

namespace game::components
{
    class CiderDashAbility final : public Ability
    {
    public:
        explicit CiderDashAbility(
            game::ArenaContext* context,
            game::entities::Entity* owner,
            const std::string& puddleTextureKey = "");

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        game::ArenaContext* context_;
        game::entities::Entity* owner_;
        std::string puddleTextureKey_;

        float cooldown_ = 4.0f;
        float cooldownTimer_ = 0.0f;
        float dashForce_ = 0.0f;
        float manaCost_ = 40.0f;

        // Mechanika ka³u¿
        float dashDuration_ = 0.15f;
        float dashTimer_ = 0.0f;
        float puddleSpawnRate_ = 0.50f; // Co ile sekund wyrzuca ka³uze podczas dasha
        float puddleTimer_ = 0.0f;

        bool isOnCooldown() const;
        void spawnPuddle();
    };
}