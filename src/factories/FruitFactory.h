#pragma once
#include <memory>
#include <string>
#include "../entities/Entity.h"
#include "../entities/EntityTypes.h"
#include "../core/ArenaContext.h"
#include "../vendor/nlohmann/json.hpp"
#include "../core/Game.h"

namespace game::factories
{
    class FruitFactory
    {
    private:
        game::ArenaContext& context;
        const nlohmann::json& config;

        // Pointers for physics component initialization
        game::Game* game;
        const sf::Image& collisionMask;
        float mapScale;

    public:
        FruitFactory(game::ArenaContext& arenaContext, const nlohmann::json& jsonConfig, game::Game* gameRef, const sf::Image& mask, float scale);
        std::unique_ptr<game::entities::Entity> createFruit(game::entities::FruitType type);
    };
}