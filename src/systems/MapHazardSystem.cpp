// --- MapHazardSystem.cpp ---

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
            config_.minSpawnTime = haz.value("minSpawnTime", 3.0f);
            config_.maxSpawnTime = haz.value("maxSpawnTime", 6.0f);
        }
    }

    void MapHazardSystem::update(float dt, game::ArenaContext& context, game::entities::Entity* player)
    {
        if (!player) return;

        hazardTimer_ -= dt;
        if (hazardTimer_ <= 0.0f)
        {
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_real_distribution<float> offsetDist(-200.0f, 200.0f);
            sf::Vector2f targetPos = player->position + sf::Vector2f(offsetDist(gen), offsetDist(gen));

            if (mapKey_ == "ChoppingBlock") spawnChoppingBlockHazard(context, targetPos);
            else if (mapKey_ == "CrisperDrawer") spawnCrisperDrawerHazard(context, targetPos);
            else if (mapKey_ == "WildOrchard") spawnWildOrchardHazard(context, targetPos);

            std::uniform_real_distribution<float> timeDist(config_.minSpawnTime, config_.maxSpawnTime);
            hazardTimer_ = timeDist(gen);
        }
    }

    void MapHazardSystem::spawnChoppingBlockHazard(game::ArenaContext& context, const sf::Vector2f& targetPos)
    {
        sf::Vector2f startPos = { targetPos.x, targetPos.y - config_.dropHeight };
        context.bullets.emplace_back(startPos, sf::Vector2f{ 0.f, 1.f });
        auto& knife = context.bullets.back();

        knife.setupDropFromSky(targetPos, config_.dropHeight, config_.speed);

        knife.setDamage(config_.damage);
        knife.setFriendly(false);
        knife.setStatusEffect(game::components::StatusEffect::None);

        knife.setSplashKeyBase("");

        auto& rm = game::core::ResourceManager::get();
        if (rm.hasTexture("hazard_knife")) {
            auto tex = rm.getTextureShared("hazard_knife");
            knife.setAnimation(tex, 1, 1.0f, { static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y) });
            knife.setSpriteScale(0.2f, 0.2f);
            knife.enableShadow(20.0f);
            knife.setImpactSound("knife_hit");
        }
        else {
            knife.setAppearance(config_.radius, sf::Color(180, 180, 180));
        }
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
            icicle.setSpriteScale(3.5f, 3.5f);
			icicle.setFriendly(false); // hazards should not be friendly to the player
            icicle.enableShadow(16.0f);
            icicle.setImpactSound("icicle_shatter");
        }
        else {
            // Fallback
            icicle.setAppearance(config_.radius, sf::Color(100, 200, 255));
        }

        context.bullets.push_back(icicle);
    }

    void MapHazardSystem::spawnWildOrchardHazard(game::ArenaContext& context, const sf::Vector2f& targetPos)
    {
        game::components::Bullet spore({ targetPos.x, targetPos.y - config_.dropHeight }, { 0.f, 1.f });
        //spore.setupParabolic({ targetPos.x, targetPos.y - config_.dropHeight }, targetPos, config_.speed);
        spore.setupDropFromSky(targetPos, config_.dropHeight, config_.speed);

        spore.setDamage(config_.damage);
        spore.setStatusEffect(game::components::StatusEffect::SporePoison);
        spore.setFriendly(false);
        spore.setSplashKeyBase("hazard_spore_splash"); // for Combat System

        auto& rm = game::core::ResourceManager::get();
        if (rm.hasTexture("hazard_spore"))
        {
            auto tex = rm.getTextureShared("hazard_spore");
            auto size = tex->getSize();
            spore.setAnimation(tex, 1, 1.0f, { static_cast<int>(size.x), static_cast<int>(size.y) });
            spore.setSpriteScale(2.5f, 2.5f);
            spore.setWobble(true, false);
            spore.enableShadow(16.0f);
            spore.setImpactSound("spore_splat");
        }
        else
        {
            // Fallback
            spore.setAppearance(config_.radius, sf::Color(130, 255, 50));
        }

        context.bullets.push_back(spore);
    }
}