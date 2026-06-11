#include "MapHazardSystem.h"
#include "../core/ArenaContext.h"
#include "../core/ResourceManager.h"
#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../components/ProjectileComponent.h"
#include "../components/AoEComponent.h"
#include <random>

namespace game::systems
{
    MapHazardSystem::MapHazardSystem(const std::string& mapKey, const nlohmann::json& mapConfig)
        : mapKey_(mapKey)
    {
        float mapDmgMulti = mapConfig.value("damageMultiplier", 1.0f);
        if (mapConfig.contains("hazard"))
        {
            const auto& haz = mapConfig["hazard"];
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
        if (!player || player->isDead()) return;

        hazardTimer_ -= dt;
        if (hazardTimer_ <= 0.0f)
        {
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_real_distribution<float> offsetDist(-200.0f, 200.0f);

            auto* player_transform = player->getComponent<game::components::TransformComponent>();
            if (!player_transform) return;

            sf::Vector2f targetPos = player_transform->position + sf::Vector2f(offsetDist(gen), offsetDist(gen));

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

        auto hazardEntity = std::make_unique<game::entities::Entity>();
        //if (auto* transform = hazardEntity->getComponent<game::components::TransformComponent>()) transform->position = startPos;
        hazardEntity->addComponent(std::make_unique<game::components::TransformComponent>(startPos));

        auto knife = std::make_unique<game::components::ProjectileComponent>(startPos, sf::Vector2f{ 0.f, 1.f });

        knife->setupDropFromSky(targetPos, config_.dropHeight, config_.speed);
        knife->setDamage(config_.damage);
        knife->setFriendly(false);
        knife->setStatusEffect(game::components::StatusEffect::None);
        knife->setSplashKeyBase("");

        auto& rm = game::core::ResourceManager::get();
        if (rm.hasTexture("hazard_knife")) {
            auto tex = rm.getTextureShared("hazard_knife");
            knife->setAnimation(tex, 1, 1.0f, { static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y) });
            knife->setSpriteScale(0.2f, 0.2f);
            knife->enableShadow(20.0f);
            knife->setImpactSound("knife_hit");
        }
        else {
            knife->setAppearance(config_.radius, sf::Color(180, 180, 180));
        }

        hazardEntity->addComponent(std::move(knife));
        context.spawnEntity(std::move(hazardEntity));
    }

    void MapHazardSystem::spawnCrisperDrawerHazard(game::ArenaContext& context, const sf::Vector2f& targetPos)
    {
        sf::Vector2f startPos = { targetPos.x, targetPos.y - config_.dropHeight };

        auto hazardEntity = std::make_unique<game::entities::Entity>();
        //if (auto* transform = hazardEntity->getComponent<game::components::TransformComponent>()) transform->position = startPos;
        hazardEntity->addComponent(std::make_unique<game::components::TransformComponent>(startPos));

        auto icicle = std::make_unique<game::components::ProjectileComponent>(startPos, sf::Vector2f{ 0.f, 1.f });

        icicle->setupDropFromSky(targetPos, config_.dropHeight, config_.speed);
        icicle->setDamage(config_.damage);
        icicle->setStatusEffect(game::components::StatusEffect::IceShatter);
        icicle->setFriendly(false);

        auto& rm = game::core::ResourceManager::get();
        if (rm.hasTexture("hazard_icicle")) {
            auto tex = rm.getTextureShared("hazard_icicle");
            auto size = tex->getSize();
            icicle->setAnimation(tex, 1, 1.0f, { static_cast<int>(size.x), static_cast<int>(size.y) });
            icicle->setSpriteScale(3.5f, 3.5f);
            icicle->enableShadow(16.0f);
            icicle->setImpactSound("icicle_shatter");
        }
        else {
            icicle->setAppearance(config_.radius, sf::Color(100, 200, 255));
        }

        hazardEntity->addComponent(std::move(icicle));
        context.spawnEntity(std::move(hazardEntity));
    }

    void MapHazardSystem::spawnWildOrchardHazard(game::ArenaContext& context, const sf::Vector2f& targetPos)
    {
        sf::Vector2f startPos = { targetPos.x, targetPos.y - config_.dropHeight };

        auto hazardEntity = std::make_unique<game::entities::Entity>();
        //if (auto* transform = hazardEntity->getComponent<game::components::TransformComponent>()) transform->position = startPos;
        hazardEntity->addComponent(std::make_unique<game::components::TransformComponent>(startPos));

        auto spore = std::make_unique<game::components::ProjectileComponent>(startPos, sf::Vector2f{ 0.f, 1.f });

        spore->setupDropFromSky(targetPos, config_.dropHeight, config_.speed);
        spore->setDamage(config_.damage);
        spore->setStatusEffect(game::components::StatusEffect::SporePoison);
        spore->setFriendly(false);
        spore->setSplashKeyBase("hazard_spore_splash");

        auto& rm = game::core::ResourceManager::get();
        if (rm.hasTexture("hazard_spore"))
        {
            auto tex = rm.getTextureShared("hazard_spore");
            auto size = tex->getSize();
            spore->setAnimation(tex, 1, 1.0f, { static_cast<int>(size.x), static_cast<int>(size.y) });
            spore->setSpriteScale(2.5f, 2.5f);
            spore->setWobble(true, false);
            spore->enableShadow(16.0f);
            spore->setImpactSound("spore_splat");
        }
        else
        {
            spore->setAppearance(config_.radius, sf::Color(130, 255, 50));
        }

        hazardEntity->addComponent(std::move(spore));
        context.spawnEntity(std::move(hazardEntity));
    }
}