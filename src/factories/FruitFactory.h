// --- FruitFactory.h --- 

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "../entities/Entity.h"
#include "../entities/EntityTypes.h"
#include "../core/ArenaContext.h"
#include "../vendor/nlohmann/json.hpp"

namespace game::components { class AbilityComponent; }

namespace game::factories
{
    class FruitFactory
    {
    private:
        game::ArenaContext& context;
        const nlohmann::json& config;

        const sf::Image& collisionMask;
        float mapScale;

        std::vector<std::unique_ptr<game::entities::Entity>>& enemies;

    private:
        std::string fruitTypeToString(game::entities::FruitType type);

        void attachAbility(
            game::components::AbilityComponent* abilities,
            const std::string& abilityName,
            game::entities::Entity* entity,
            const nlohmann::json& fruitData,
            const std::string & key);

    public:
        FruitFactory(
            game::ArenaContext& arenaContext,
            const nlohmann::json& jsonConfig,
            const sf::Image& mask,
            float scale,
            std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef
        );

        std::unique_ptr<game::entities::Entity> createFruit(
            game::entities::FruitType type);
    };
}