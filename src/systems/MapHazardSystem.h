// --- MapHazardSystem.h ---

#pragma once
#include <string>
#include <SFML/System/Vector2.hpp>
#include <../vendor/nlohmann/json.hpp>

namespace game { struct ArenaContext; }
namespace game::entities { class Entity; }

namespace game::systems
{
	// default configuration for hazards, can be overridden by map-specific JSON config
    struct HazardConfig
    {
        float damage = 25.0f;
        float speed = 800.0f;
        float dropHeight = 1000.0f;
        float radius = 10.0f;
    };

    class MapHazardSystem final
    {
    public:
        MapHazardSystem(const std::string& mapKey, const nlohmann::json& mapConfig);

        void update(float dt, game::ArenaContext& context, game::entities::Entity* player);

    private:
        std::string mapKey_;
        float hazardTimer_ = 5.0f;

        HazardConfig config_;

        void spawnChoppingBlockHazard(game::ArenaContext& context, const sf::Vector2f& targetPos);
        void spawnCrisperDrawerHazard(game::ArenaContext& context, const sf::Vector2f& targetPos);
        void spawnWildOrchardHazard(game::ArenaContext& context, const sf::Vector2f& targetPos);
    };
}