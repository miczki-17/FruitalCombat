// --- CollisionSystem.cpp ---

#include "CollisionSystem.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"
#include "../components/ProjectileComponent.h"
#include "../components/StatsComponent.h"
#include "../components/TransformComponent.h"
#include "../components/JuiceComponent.h"
#include <cmath>

namespace game::systems
{
    CollisionSystem::CollisionSystem(game::ArenaContext& context, std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef)
        : context_(context), enemies_(enemiesRef)
    {
    }

    void CollisionSystem::updateJuiceCollection(game::entities::Entity* player, float dt)
    {
        if (!player || player->isDead()) return;

        auto* player_transform = player->getComponent<game::components::TransformComponent>();
        if (!player_transform) return;

        auto& entities = context_.entities;

        for (auto& entity : entities)
        {
            auto* juice = entity->getComponent<game::components::JuiceComponent>();
            if (!juice || juice->isCollected) continue;

            auto* juice_transform = entity->getComponent<game::components::TransformComponent>();
            if (!juice_transform) continue;

            sf::Vector2f diff = player_transform->position - juice_transform->position;
            float dist = diff.length(); // SFML 3

            if (dist < 180.0f && dist > 0.001f)
            {
                // Przyspieszone "magnesowanie" do gracza
                juice_transform->position += (diff / dist) * 350.0f * dt;
            }

            if (dist < 30.0f)
            {
                juice->isCollected = true;
            }
        }
    }

    void CollisionSystem::updateBulletIntersections(float dt, const sf::Image& collisionMask, float mapScale)
    {
        auto& entities = context_.entities;

        for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i)
        {
            // Interesuj¹ nas tylko encje posiadaj¹ce komponent pocisku
            auto* proj = entities[i]->getComponent<game::components::ProjectileComponent>();
            if (!proj) continue;

            proj->update(dt, collisionMask, mapScale);

            if (!proj->getIsActive()) continue;

            sf::Vector2f bulletPos = proj->getPosition();

            for (auto& enemy : enemies_)
            {
                if (enemy->isDead()) continue;

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