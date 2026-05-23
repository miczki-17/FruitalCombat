// --- ArenaContext.h ---


#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "../projectiles/Bullet.h"
#include "../effects/AcidSplash.h"
#include "../effects/FloatingText.h"
#include "../entities/JuiceDrop.h"

namespace game
{
    // Gameplay Area-of-Effect Zone definition (Replaces unsafe global file statics)
    struct AoEZone {
        sf::CircleShape shape;
        float lifetime = 4.0f;
        float maxLifetime = 4.0f;
        float radius = 55.0f;
    };

    // Gameplay walk particles
    struct WalkDust {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float lifetime;
        float maxLifetime;
        float size;
        sf::Color color;

        WalkDust(sf::Vector2f pos, sf::Vector2f vel, float life, float s, sf::Color c)
            : position(pos), velocity(vel), lifetime(life), maxLifetime(life), size(s), color(c) {}
    };


    struct ArenaContext
    {
        // Reference container allocated inside PlayingState
        std::vector<game::components::Bullet>* bullets = nullptr;

        // Container containing short-lived decorative dynamic splash sprites
        std::vector<game::effects::AcidSplash> acidSplashes;

        // Active damage/hazard handling circles
        std::vector<AoEZone> dynamicZones;

        // Visual textures preallocated at arena initialization
        std::vector<std::shared_ptr<sf::Texture>> splashTextures;

        std::vector<game::effects::FloatingText> damageTexts;
        std::vector<game::entities::JuiceDrop> juiceDrops;

        // walk particles
        std::vector<WalkDust> walkParticles;
        sf::Color currentMapDustColor = sf::Color(200, 200, 200, 150);
    };
}