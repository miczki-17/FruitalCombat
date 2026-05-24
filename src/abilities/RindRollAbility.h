#pragma once
#include "Ability.h"
#include "../core/ArenaContext.h"
#include <vector>
#include <memory>

namespace game::entities { class Entity; }

namespace game::components
{
    class RindRollAbility : public Ability
    {
    private:
        game::entities::Entity* entity;
        game::ArenaContext* context;
        std::vector<std::unique_ptr<game::entities::Entity>>* enemies;
        game::entities::Entity* singleTarget;

        float cooldown = 5.0f;
        float currentTimer = 0.0f;
        float rollDuration = 1.0f;
        float rollSpeed = 450.0f;

        // --- NOWE ZMIENNE ODRZUTU ---
        float knockbackRadius;
        float knockbackForce;

    public:
        // Konstruktor dla gracza
        RindRollAbility(game::entities::Entity* targetEntity, game::ArenaContext* ctx, std::vector<std::unique_ptr<game::entities::Entity>>* targetsList, float kRadius = 120.0f, float kForce = 350.0f);

        // Konstruktor dla mutanta
        RindRollAbility(game::entities::Entity* targetEntity, game::ArenaContext* ctx, game::entities::Entity* playerTarget, float kRadius = 80.0f, float kForce = 200.0f);

        void update(float dt) override;
        void execute(const sf::Vector2f& startPos, const sf::Vector2f& targetWorldPos, const sf::Vector2f& shooterVelocity) override;
    };
}