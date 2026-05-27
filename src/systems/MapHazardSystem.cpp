// ==========================================
// systems/MapHazardSystem.cpp
// ==========================================
#include "MapHazardSystem.h"
#include "../core/ArenaContext.h"
#include "../core/ResourceManager.h"
#include "../entities/Entity.h"
#include "../projectiles/Bullet.h"
#include <random>

namespace game::systems
{
    MapHazardSystem::MapHazardSystem(const std::string& mapKey, const nlohmann::json& mapConfig)
        : mapKey_(mapKey)
    {
		// 1. get global map multipliers (like damage multiplier) that apply to all hazards on this map
        float mapDmgMulti = mapConfig.value("damageMultiplier", 1.0f);

		// 2. load specific hazard parameters if they exist, otherwise use defaults
        if (mapConfig.contains("hazard"))
        {
            const auto& haz = mapConfig["hazard"];
            // damage = base val * map multiplier
            config_.damage = haz.value("damage", 25.0f) * mapDmgMulti;
            config_.speed = haz.value("speed", 800.0f);
            config_.dropHeight = haz.value("dropHeight", 1000.0f);
            config_.radius = haz.value("radius", 10.0f);
        }
    }

    void MapHazardSystem::update(float dt, game::ArenaContext& context, game::entities::Entity* player)
    {
        if (!player) return;

        hazardTimer_ -= dt;
        if (hazardTimer_ <= 0.0f)
        {
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_real_distribution<float> offsetDist(-300.0f, 300.0f);
            sf::Vector2f targetPos = player->position + sf::Vector2f(offsetDist(gen), offsetDist(gen));

            if (mapKey_ == "ChoppingBlock") spawnChoppingBlockHazard(context, targetPos);
            else if (mapKey_ == "CrisperDrawer") spawnCrisperDrawerHazard(context, targetPos);
            else if (mapKey_ == "WildOrchard") spawnWildOrchardHazard(context, targetPos);

            std::uniform_real_distribution<float> timeDist(3.0f, 6.0f);
            hazardTimer_ = timeDist(gen);
        }
    }

    void MapHazardSystem::spawnChoppingBlockHazard(game::ArenaContext& context, const sf::Vector2f& targetPos)
    {
        game::components::Bullet cleaver({ targetPos.x, targetPos.y - config_.dropHeight }, { 0.f, 1.f });
        cleaver.setupParabolic({ targetPos.x, targetPos.y - config_.dropHeight }, targetPos, config_.speed);

        cleaver.setAppearance(config_.radius, sf::Color(180, 180, 180));
        cleaver.setDamage(config_.damage);

        context.bullets.push_back(cleaver);
    }

    void MapHazardSystem::spawnCrisperDrawerHazard(game::ArenaContext& context, const sf::Vector2f& targetPos)
    {
        game::components::Bullet icicle({ targetPos.x, targetPos.y - config_.dropHeight }, { 0.f, 1.f });

        icicle.setupDropFromSky(targetPos, config_.dropHeight, config_.speed);
        icicle.setDamage(config_.damage);
        icicle.setStatusEffect(game::components::StatusEffect::IceShatter);

        auto& rm = game::core::ResourceManager::get();
        if (rm.hasTexture("hazard_icicle")) {
            auto tex = rm.getTextureShared("hazard_icicle");
            auto size = tex->getSize();
            icicle.setAnimation(tex, 1, 1.0f, { static_cast<int>(size.x), static_cast<int>(size.y) });
        }
        else {
            icicle.setAppearance(config_.radius, sf::Color(100, 200, 255));
        }

        context.bullets.push_back(icicle);
    }

    void MapHazardSystem::spawnWildOrchardHazard(game::ArenaContext& context, const sf::Vector2f& targetPos)
    {
        game::components::Bullet spore({ targetPos.x, targetPos.y - config_.dropHeight }, { 0.f, 1.f });
        spore.setupParabolic({ targetPos.x, targetPos.y - config_.dropHeight }, targetPos, config_.speed);

        spore.setAppearance(config_.radius, sf::Color(130, 255, 50));
        spore.setDamage(config_.damage);
        spore.setStatusEffect(game::components::StatusEffect::Poison);

        context.bullets.push_back(spore);
    }
}