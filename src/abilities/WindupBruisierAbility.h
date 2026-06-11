#pragma once
#include "Ability.h"
#include "../entities/Entity.h"
#include "../core/ArenaContext.h"
#include "../vendor/nlohmann/json.hpp"

namespace game::components
{
    enum class BruiserState { Idle, Signaling, JumpingUp, FallingDown, Recovering };

    class WindupBruiserAbility : public Ability
    {
    public:
        // Konstruktor przyjmuje opcjonalny config, ?eby ?atwo go poda? z fabryki
        WindupBruiserAbility(game::ArenaContext* context,
            game::entities::Entity* owner,
            game::entities::Entity* targetPlayer,
            const nlohmann::json& config = nlohmann::json::object());

        void update(float deltaTime) override;

        void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) override;

    private:
        game::ArenaContext* context_;
        game::entities::Entity* owner_;
        game::entities::Entity* targetPlayer_;

        // Parametry z JSON'a
        float triggerDistance_;
        float signalTime_;
        float jumpHeight_;
        float jumpSpeed_;
        float fallSpeed_;
        float recoveryTime_;
        float damageRadius_;
        float damage_;

        // Zmienne stanu
        BruiserState state_ = BruiserState::Idle;
        float timer_ = 0.0f;

        sf::Vector2f originalPosition_;

        // Pr?dko?? przesuwania si? w locie
        sf::Vector2f leapVelocity_;
    };
}