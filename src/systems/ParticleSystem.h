// --- ParticleSystem.h ---

#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace game
{
    struct ArenaContext;
}

namespace game::entities
{
    class Entity;
}

namespace game::systems
{
    class ParticleSystem final
    {
    public:
        explicit ParticleSystem(game::ArenaContext& context);

        void updateEffects(float dt);

        void updateParticles(game::entities::Entity* player, float dt, sf::Vector2f& lastPlayerPos, float& playerDustSpawnTimer);

    private:
        game::ArenaContext& context_;
    };
}