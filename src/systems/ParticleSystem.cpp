// --- ParticleSystem.cpp ---

#include "ParticleSystem.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../components/TextComponent.h"
#include "../components/LifespanComponent.h"
#include "../components/PopAnimationComponent.h"
#include <algorithm>
#include <random>
#include <cmath>

namespace game::systems
{
    ParticleSystem::ParticleSystem(game::ArenaContext& context)
        : context_(context)
    {
    }

    void ParticleSystem::updateEffects(float dt)
    {
        // 1. Aktualizacja stref AoE
        auto& zones = context_.zones;
        for (int i = static_cast<int>(zones.size()) - 1; i >= 0; --i)
        {
            zones[i].lifetime -= dt;
            if (zones[i].lifetime <= 0.0f)
            {
                zones.erase(zones.begin() + i);
            }
        }

        // 2. O¿ywianie i sprz¹tanie encji wizualnych (ECS)
        auto& entities = context_.entities;
        for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i)
        {
            auto& entity = entities[i];

            if (auto* pop = entity->getComponent<game::components::PopAnimationComponent>()) {
                pop->update(dt);
            }

            if (auto* lifespan = entity->getComponent<game::components::LifespanComponent>()) {
                lifespan->update(dt);
            }

            if (auto* text = entity->getComponent<game::components::TextComponent>()) {
                if (auto* transform = entity->getComponent<game::components::TransformComponent>()) {
                    transform->position += transform->velocity * dt;
                }
                text->update(dt);
            }

            // Œmieciarka: usuwa encje przeznaczone do usuniêcia
            if (entity->isPendingDestroy) {
                entities.erase(entities.begin() + i);
            }
        }
    }

    void ParticleSystem::updateParticles(game::entities::Entity* player, float dt, sf::Vector2f& lastPlayerPos, float& playerDustSpawnTimer)
    {
        if (!player) return;

        auto& particles = context_.walkParticles;
        auto* player_transform = player->getComponent<game::components::TransformComponent>();
        if (!player_transform) return;

        for (int i = static_cast<int>(particles.size()) - 1; i >= 0; --i)
        {
            particles[i].lifetime -= dt;
            if (particles[i].lifetime <= 0.0f)
            {
                particles.erase(particles.begin() + i);
            }
            else
            {
                particles[i].position += particles[i].velocity * dt;
                particles[i].velocity *= 0.95f;
            }
        }

        sf::Vector2f moveDelta = player_transform->position - lastPlayerPos;
        bool isMoving = (moveDelta.x * moveDelta.x + moveDelta.y * moveDelta.y) > 0.1f;
        lastPlayerPos = player_transform->position;

        if (isMoving)
        {
            playerDustSpawnTimer -= dt;
            if (playerDustSpawnTimer <= 0.0f)
            {
                playerDustSpawnTimer = 0.06f;

                std::random_device rd; std::mt19937 gen(rd());
                std::uniform_real_distribution<float> offsetDist(-12.0f, 12.0f);
                std::uniform_real_distribution<float> velDist(-25.0f, 25.0f);
                std::uniform_real_distribution<float> sizeDist(4.0f, 8.0f);

                particles.emplace_back(
                    player_transform->position + sf::Vector2f(offsetDist(gen), offsetDist(gen) + 18.0f),
                    sf::Vector2f(velDist(gen), velDist(gen) - 10.0f),
                    0.4f, sizeDist(gen), context_.mapDustColor
                );
            }
        }
        else
        {
            playerDustSpawnTimer = 0.0f;
        }
    }
}