// --- RindRollAbility.h ---

#pragma once
#include "Ability.h"
#include "../core/ArenaContext.h"
#include <vector>
#include <memory>

namespace game { struct ArenaContext; }
namespace game::entities { class Entity; }

namespace game::components
{
    class RindRollAbility : public Ability
    {
    private:
        game::entities::Entity* owner_;
        game::ArenaContext* context_;
        std::vector<std::unique_ptr<game::entities::Entity>>* enemies_;
        game::entities::Entity* singleTarget_;

        float manaCost_ = 40.0f;

        float cooldown_ = 5.0f;
        float currentTimer_ = 0.0f;
        float rollDuration_ = 1.0f;
        float rollSpeed_ = 450.0f;

        float knockbackRadius_;
        float knockbackForce_;

    public:
        // Konstruktor dla gracza (bije wielu wrogow)
        RindRollAbility(
            game::entities::Entity* owner,
            game::ArenaContext* ctx,
            std::vector<std::unique_ptr<game::entities::Entity>>* targetsList,
            float kRadius = 120.0f,
            float kForce = 350.0f);

        // Konstruktor dla mutanta (bije tylko gracza)
        RindRollAbility(
            game::entities::Entity* owner,
            game::ArenaContext* ctx,
            game::entities::Entity* playerTarget,
            float kRadius = 80.0f,
            float kForce = 200.0f);

        void update(float dt) override;

        void execute(
            const sf::Vector2f& startPos,
            const sf::Vector2f& targetWorldPos,
            const sf::Vector2f& shooterVelocity) override;
    };
}