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

    void CollisionSystem::updateBulletIntersections(float dt, const sf::Image& collisionMask, float mapScale, game::entities::Entity* player)
    {
        auto& entities = context_.entities;

        for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i)
        {
            auto* proj = entities[i]->getComponent<game::components::ProjectileComponent>();
            if (!proj) continue;

            proj->update(dt, collisionMask, mapScale);

            if (!proj->getIsActive()) continue;

            sf::Vector2f bulletPos = proj->getPosition();

            // 1. Kule uderzaj?ce w PRZECIWNIKÓW (Strzela Gracz)
            if (proj->getIsFriendly())
            {
                for (auto& enemy : enemies_)
                {
                    if (enemy->isDead()) continue;

                    auto* enemy_transform = enemy->getComponent<game::components::TransformComponent>();
                    if (!enemy_transform) continue;

                    float combinedRadius = proj->getRadius() + 30.0f; // 30.0f -> szacowany hitbox przeciwnika
                    sf::Vector2f diff = bulletPos - enemy_transform->position;

                    if (diff.lengthSquared() < (combinedRadius * combinedRadius))
                    {
                        proj->destroy();
                        break;
                    }
                }
            }
            // 2. Kule uderzaj?ce w GRACZA (Strzelaj? Potwory)
            else if (!proj->getIsFriendly() && player && !player->isDead())
            {
                if (auto* player_transform = player->getComponent<game::components::TransformComponent>())
                {
                    float combinedRadius = proj->getRadius() + 25.0f; // 25.0f -> szacowany hitbox gracza
                    sf::Vector2f diff = bulletPos - player_transform->position;

                    if (diff.lengthSquared() < (combinedRadius * combinedRadius))
                    {
                        proj->destroy();
                    }
                }
            }
        }
    }
}