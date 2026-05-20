#pragma once
#include <memory>
#include <map>
#include <string>
#include <functional>
#include "../entities/player/Player.h"
#include "../entities/EntityTypes.h"
#include "../core/ArenaContext.h"
#include "../vendor/nlohmann/json.hpp"

namespace game::factories
{
    class FruitFactory
    {
    public:
        using AbilityCreator = std::function<std::unique_ptr<game::components::Ability>(
            game::ArenaContext& context,
            game::entities::Player* player,
            const nlohmann::json& fruitData,
            const std::string& abilityName)>;

    private:
        game::ArenaContext& context;
        const nlohmann::json& config;

        // Registry table enabling dynamic new extensions without touching central logic
        static std::map<std::string, AbilityCreator> abilityRegistry;

    public:
        FruitFactory(game::ArenaContext& arenaContext, const nlohmann::json& jsonConfig);

        // Call this from anywhere to add a completely new ability package to the engine runtime
        //static void registerAbility(const std::string& name, AbilityCreator creator);

        std::unique_ptr<game::entities::Player> createFruit(game::entities::FruitType type);
    };
}