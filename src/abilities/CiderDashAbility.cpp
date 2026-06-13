// --- CiderDashAbility.cpp ---

#include "CiderDashAbility.h"

#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../components/PlayerInputComponent.h"
#include "../components/ProjectileComponent.h"
#include "../components/LifespanComponent.h"
#include "../components/StatsComponent.h"
#include "../core/ResourceManager.h"
#include "../components/AoEComponent.h"
#include "../components/SpriteComponent.h"

#include <cmath>

namespace game::components
{
    CiderDashAbility::CiderDashAbility(
        game::ArenaContext* context,
        game::entities::Entity* owner,
        const std::string& puddleTextureKey,
        std::string sourceName)
        : Ability(std::move(sourceName)),
        context_(context),
        owner_(owner),
        puddleTextureKey_(puddleTextureKey)
    {
    }

    void CiderDashAbility::update(float deltaTime)
    {
        if (cooldownTimer_ > 0.0f)
        {
            cooldownTimer_ -= deltaTime;
        }

        // Jeœli gracz w³aœnie dashuje, "upuszczamy" ka³u¿e
        if (dashTimer_ > 0.0f)
        {
            dashTimer_ -= deltaTime;
            puddleTimer_ -= deltaTime;

            if (puddleTimer_ <= 0.0f)
            {
                spawnPuddle();
                puddleTimer_ = puddleSpawnRate_;
            }
        }
    }

    void CiderDashAbility::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (!owner_ || isOnCooldown()) return;

        auto* stats = owner_->getComponent<StatsComponent>();
        if (stats) {
            if (owner_->getComponent<PlayerInputComponent>() && stats->getMana() < manaCost_) {
                return;
            }
        }

        auto* owner_transform = owner_->getComponent<TransformComponent>();
        if (!owner_transform) return;

        // Obliczanie kierunku dasha (do myszki)
        sf::Vector2f aimDirection = targetPosition - origin;
        float aimLength = std::sqrt(aimDirection.x * aimDirection.x + aimDirection.y * aimDirection.y);

        sf::Vector2f dashDirection(1.0f, 0.0f); // Default
        if (aimLength > 0.001f) {
            dashDirection = aimDirection / aimLength;
        }

        // Nak³adamy ogromn¹ si³ê, jak w klasycznym Dashu
        owner_transform->velocity += (dashDirection * dashForce_);
        owner_transform->actionTimer = dashDuration_;

        // Aktywujemy tryb zostawiania ka³u¿!
        dashTimer_ = dashDuration_;
        puddleTimer_ = 0.0f; // Od razu rzuca pierwsz¹ ka³u¿ê pod siebie
        cooldownTimer_ = cooldown_;

        if (stats && owner_->getComponent<PlayerInputComponent>()) {
            stats->consumeMana(manaCost_);
        }
    }

    bool CiderDashAbility::isOnCooldown() const
    {
        return cooldownTimer_ > 0.0f;
    }

    void CiderDashAbility::spawnPuddle()
    {
        if (!context_ || !owner_) return;

        auto* ownerTransform = owner_->getComponent<TransformComponent>();
        if (!ownerTransform) return;

        auto puddleEntity = std::make_unique<game::entities::Entity>();

        if (auto* transform = puddleEntity->getComponent<TransformComponent>())
        {
            transform->position = ownerTransform->position;
        }

        auto puddleAoE = std::make_unique<game::components::AoEComponent>(
            90.0f,                              // radius
            sf::Color(255, 220, 100, 180),      // fallback color
            0.0f,                               // dps
            false,
            0.0f,
            true,                               // applies slow
            0.5f,                               // 50% speed
            true,                               // is friendly
            sourceName_                         // DZIEDZICZONA NAZWA
            );

        if (!puddleTextureKey_.empty())
        {
            auto tex = game::core::ResourceManager::get().getTextureShared(puddleTextureKey_);

            if (tex)
            {
                puddleAoE->setTexture(tex);

                if (puddleAoE->sprite.has_value())
                {
                    puddleAoE->sprite->setColor(
                        sf::Color(255, 235, 120, 190)
                    );
                }
            }
        }

        puddleEntity->addComponent(std::move(puddleAoE));
        puddleEntity->addComponent(std::make_unique<LifespanComponent>(4.0f, true));

        context_->spawnEntity(std::move(puddleEntity));
    }
}