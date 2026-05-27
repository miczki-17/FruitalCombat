// --- ArenaContext.h ---

#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <SFML/Graphics.hpp>

#include "../entities/Entity.h"
#include "../projectiles/Bullet.h"
#include "../effects/AcidSplash.h"
#include "../effects/FloatingText.h"
#include "../entities/JuiceDrop.h"

namespace game::components { class StatsComponent; }

namespace game
{
    struct AoEZone
    {
        sf::CircleShape shape;
        float lifetime = 4.0f;
        float maxLifetime = 4.0f;
        float radius = 55.0f;
        float dps = 0.0f;
        bool appliesPoison = false;
        float poisonDps = 0.0f;
        bool appliesSlow = false;
        float slowMultiplier = 1.0f;
    };

    struct WalkDust
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float lifetime;
        float maxLifetime;
        float size;
        sf::Color color;

        WalkDust(sf::Vector2f pos, sf::Vector2f vel, float life, float s, sf::Color c)
            : position(pos), velocity(vel), lifetime(life), maxLifetime(life), size(s), color(c) {
        }
    };

    struct ArenaContext final
    {
        // --- DOCELOWA PULA ECS ---
        std::vector<std::unique_ptr<game::entities::Entity>> entities;

        // --- LEGACY WEKTORY ---
        std::vector<game::components::Bullet> bullets;
        std::vector<game::effects::AcidSplash> acidSplashes;
        std::vector<AoEZone> zones;
        std::vector<std::shared_ptr<sf::Texture>> splashTextures;
        std::vector<game::effects::FloatingText> floatingTexts;
        std::vector<game::entities::JuiceDrop> juiceDrops;
        std::vector<WalkDust> walkParticles;

        sf::Color mapDustColor = sf::Color(200, 200, 200, 150);
        game::components::StatsComponent* playerStats = nullptr;
    };
}