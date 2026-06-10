// --- CollisionSystem.cpp ---

#include "CollisionSystem.h"
#include "../core/ArenaContext.h"
#include "../core/Game.h"
#include "../entities/Entity.h"
#include "../components/ProjectileComponent.h"
#include "../components/StatsComponent.h"
#include "../components/TransformComponent.h"
#include "../components/PickupComponent.h"
#include <cmath>

namespace game::systems
{
    CollisionSystem::CollisionSystem(game::ArenaContext& context, std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef)
        : context_(context), enemies_(enemiesRef)
    {
    }

    void CollisionSystem::updatePickups(game::entities::Entity* player, float dt)
    {
        // 1. Zabezpieczamy pozycj? gracza (ale NIE przerywamy funkcji je?li gracz nie ?yje)
        auto* player_transform = (player && !player->isDead()) ? player->getComponent<game::components::TransformComponent>() : nullptr;

        for (auto& entity : context_.entities)
        {
            auto* pickup = entity->getComponent<game::components::PickupComponent>();
            if (!pickup) continue; // Ignoruj to, co nie jest pickupem

            // --- TO O?YWIA SOK I APTECZKI (Fizyka i Skala z 0 do 1) ---
            pickup->update(dt);

            // Je?li przedmiot zosta? ju? zebrany, albo gracz nie ?yje - pomijamy tylko magnesowanie
            if (pickup->isCollected || !player_transform) continue;

            auto* pickup_transform = entity->getComponent<game::components::TransformComponent>();
            if (!pickup_transform) continue;

            sf::Vector2f diff = player_transform->position - pickup_transform->position;
            float dist = diff.length();

            if (dist < 180.0f && dist > 0.001f) {
                // Przyspieszone "magnesowanie"
                pickup_transform->position += (diff / dist) * 350.0f * dt;
            }

            if (dist < 30.0f) {
                pickup->isCollected = true;
            }
        }
    }

    void CollisionSystem::updateBulletIntersections(float dt, const sf::Image& collisionMask, float mapScale)
    {
        auto& entities = context_.entities;

        for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i)
        {
            // Interesuj? nas tylko encje posiadaj?ce komponent pocisku
            auto* proj = entities[i]->getComponent<game::components::ProjectileComponent>();
            if (!proj) continue;

            proj->update(dt, collisionMask, mapScale);

            if (!proj->getIsActive()) continue;

            sf::Vector2f bulletPos = proj->getPosition();

            for (auto& enemy : enemies_)
            {
                if (enemy->isDead()) continue;

                if (!proj->getIsFriendly()) continue;

                auto* enemy_transform = enemy->getComponent<game::components::TransformComponent>();
                if (!enemy_transform) continue;

                float combinedRadius = proj->getRadius() + 30.0f;
                sf::Vector2f diff = bulletPos - enemy_transform->position;

                if (diff.lengthSquared() < (combinedRadius * combinedRadius))
                {
                    proj->destroy();
                    break;
                }
            }
        }
    }
}