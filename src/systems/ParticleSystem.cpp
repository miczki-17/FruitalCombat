// --- ParticleSystem.cpp ---

#include "ParticleSystem.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../components/TextComponent.h"
#include "../components/LifespanComponent.h"
#include "../components/PopAnimationComponent.h"
#include "../components/ParticleComponent.h"
#include "../components/JuiceComponent.h"
#include "../components/MovementComponent.h"
#include "../components/AoEComponent.h"
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
        // O¿ywianie i sprz¹tanie encji wizualnych (ECS)
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

            if (auto* juice = entity->getComponent<game::components::JuiceComponent>()) {
                juice->update(dt);
            }

            // O¿ywiamy kurz (Lokalne tarcie i zwalnianie)
            if (auto* particle = entity->getComponent<game::components::ParticleComponent>()) {
                particle->update(dt);
            }

            // O¿ywiamy AoE
            if (auto* aoe = entity->getComponent<game::components::AoEComponent>()) {
                aoe->update(dt);
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

        auto* player_transform = player->getComponent<game::components::TransformComponent>();
        if (!player_transform) return;

        // 1. Generowanie nowych czasteczek kurzu na podstawie ruchu gracza
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

                float randomSize = sizeDist(gen);
                sf::Vector2f randomOffset(offsetDist(gen), offsetDist(gen) + 18.0f);
                sf::Vector2f randomVel(velDist(gen), velDist(gen) - 10.0f);

                // --- SPAWNOWANIE ECS ---

                auto dust = std::make_unique<game::entities::Entity>();

                // 1. Transform: Pozycja startowa i predkosc
                if (auto* t = dust->getComponent<game::components::TransformComponent>()) {
                    t->position = player_transform->position + randomOffset;
                    t->velocity = randomVel;
                }

                // 2. Lifespan: Czas zycia 0.4s z plynnym zanikaniem (fadeOut = true)
                dust->addComponent(std::make_unique<game::components::LifespanComponent>(0.4f, true));

                // 3. Particle Data: Rozmiar, kolor i tarcie (wartosc 5.0f ladnie wyhamuje kurz)
                dust->addComponent(std::make_unique<game::components::ParticleComponent>(randomSize, context_.mapDustColor, 5.0f));

                context_.spawnEntity(std::move(dust));
            }
        }
        else
        {
            playerDustSpawnTimer = 0.0f;
        }
    }
}