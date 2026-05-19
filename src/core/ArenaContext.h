#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "../components/Bullet.h"
#include "../effects/AcidSplash.h"

namespace game
{
    // Gameplay Area-of-Effect Zone definition (Replaces unsafe global file statics)
    struct AoEZone {
        sf::CircleShape shape;
        float lifetime = 4.0f;
        float maxLifetime = 4.0f;
        float radius = 55.0f;
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
    };
}