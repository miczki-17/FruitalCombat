#pragma once
#include <vector>
#include <memory>
#include <random>
#include <SFML/Graphics/Image.hpp>

namespace game::entities { class Entity; }
namespace game { class ArenaContext; }

namespace game::systems
{
    class ItemSpawnSystem
    {
    public:
        ItemSpawnSystem(game::ArenaContext& context,
            std::vector<std::unique_ptr<game::entities::Entity>>& entitiesRef,
            const sf::Image& mask,
            float scale);

        void update(float deltaTime, game::entities::Entity* player);

    private:
        sf::Vector2f getRandomValidPosition();

        game::ArenaContext& context_;
        std::vector<std::unique_ptr<game::entities::Entity>>& entities_;
        const sf::Image& collisionMask_;
        float mapScale_;

        float medkitSpawnTimer_ = 45.0f;

        std::mt19937 rng_;
    };
}