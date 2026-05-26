// --- ParticleSystem.cpp ---

#include "ParticleSystem.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"
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
		// 1. Acctualization of floating texts (damage numbers, healing, etc.)
        auto& texts = context_.floatingTexts;
        for (int i = static_cast<int>(texts.size()) - 1; i >= 0; --i)
        {
            texts[i].update(dt);
            if (texts[i].isDead())
            {
                texts.erase(texts.begin() + i);
            }
        }

		// 2. Actualization of AoE zones (poison clouds, slow fields, etc.)
        auto& zones = context_.zones;
        for (int i = static_cast<int>(zones.size()) - 1; i >= 0; --i)
        {
            zones[i].lifetime -= dt;
            if (zones[i].lifetime <= 0.0f)
            {
                zones.erase(zones.begin() + i);
            }
        }

		// 3. Actualization of acid splashes (visual effect on the ground after acid attacks)
        auto& splashes = context_.acidSplashes;
        for (auto& splash : splashes)
        {
            splash.update(dt);
        }

        // Cleanup
        splashes.erase(std::remove_if(splashes.begin(), splashes.end(),
            [](const auto& s) { return !s.isActive(); }), splashes.end());
    }

    void ParticleSystem::updateParticles(game::entities::Entity* player, float dt, sf::Vector2f& lastPlayerPos, float& playerDustSpawnTimer)
    {
        if (!player) return;

        auto& particles = context_.walkParticles;

		// 1. Actualization of existing walk dust particles (movement, fading, etc.)
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
				particles[i].velocity *= 0.95f; // effect of friction slowing down the dust over time
            }
        }

		// 2. Generate new walk dust particles based on player movement
        sf::Vector2f moveDelta = player->position - lastPlayerPos;
        bool isMoving = moveDelta.lengthSquared() > 0.1f;
        lastPlayerPos = player->position;

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
                    player->position + sf::Vector2f(offsetDist(gen), offsetDist(gen) + 18.0f),
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