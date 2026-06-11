// --- CollisionSystem.h ---

#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics/Image.hpp>

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
    class CollisionSystem final
    {
    public:
        CollisionSystem(game::ArenaContext& context, std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef);

        void updatePickups(game::entities::Entity* player, float dt);
        void updateBulletIntersections(float dt, const sf::Image& collisionMask, float mapScale, game::entities::Entity* player);

    private:
        game::ArenaContext& context_;
        std::vector<std::unique_ptr<game::entities::Entity>>& enemies_;
    };
}