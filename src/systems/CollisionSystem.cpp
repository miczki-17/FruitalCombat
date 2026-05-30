// --- CollisionSystem.cpp ---

#include "CollisionSystem.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"
#include "../components/ProjectileComponent.h"
#include "../components/StatsComponent.h"
#include <cmath>

namespace game::systems
{
    CollisionSystem::CollisionSystem(game::ArenaContext& context, std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef)
        : context_(context), enemies_(enemiesRef)
    {
    }

    void CollisionSystem::updateJuiceCollection(game::entities::Entity* player, float dt)
    {
        if (!player || player->isDead) return;

        auto& drops = context_.juiceDrops;

        for (int i = static_cast<int>(drops.size()) - 1; i >= 0; --i)
        {
            sf::Vector2f diff = player->position - drops[i].position;
            float dist = diff.length();

            if (dist < 180.0f && dist > 0.001f)
            {
                drops[i].position += (diff / dist) * 350.0f * dt;
                drops[i].update(dt);
            }

            if (dist < 30.0f)
            {
                drops[i].isCollected = true;
            }
        }
    }

    void CollisionSystem::updateBulletIntersections(float dt, const sf::Image& collisionMask, float mapScale)
    {
        auto& bullets = context_.bullets;

        for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; --i)
        {
            bullets[i].update(dt, collisionMask, mapScale);

            if (!bullets[i].getIsActive()) continue;

            sf::Vector2f bulletPos = bullets[i].getPosition();

            for (auto& enemy : enemies_)
            {
                if (enemy->isDead) continue;

                float combinedRadius = bullets[i].getRadius() + 30.0f;
                sf::Vector2f diff = bulletPos - enemy->position;

                if (diff.lengthSquared() < (combinedRadius * combinedRadius))
                {
                    bullets[i].destroy();
                    break;
                }
            }
        }
    }
}