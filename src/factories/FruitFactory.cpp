#include "FruitFactory.h"

// ECS Components
#include "../abilities/ShootAbility.h"
#include "../abilities/ShotgunAbility.h"
#include "../abilities/DashAbility.h"
#include "../abilities/AcidSquirtAbility.h"
#include "../abilities/RindRollAbility.h"
#include "../components/MovementComponent.h"
#include "../components/ColliderComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/AbilityComponent.h"
#include "../components/StatsComponent.h"

#include "../core/ResourceManager.h"
#include <iostream>

namespace game::factories
{
    FruitFactory::FruitFactory(game::ArenaContext& arenaContext, const nlohmann::json& jsonConfig, game::Game* gameRef, const sf::Image& mask, float scale)
        : context(arenaContext), config(jsonConfig), game(gameRef), collisionMask(mask), mapScale(scale)
    {
    }

    std::unique_ptr<game::entities::Entity> FruitFactory::createFruit(game::entities::FruitType type)
    {
        auto entity = std::make_unique<game::entities::Entity>();
        std::string fruitKey;

        switch (type) {
        case game::entities::FruitType::Apple:      fruitKey = "Apple"; break;
        case game::entities::FruitType::Banana:     fruitKey = "Banana"; break;
        case game::entities::FruitType::Orange:     fruitKey = "Orange"; break;
        case game::entities::FruitType::Cherry:     fruitKey = "Cherry"; break;
        case game::entities::FruitType::Strawberry: fruitKey = "Strawberry"; break;
        case game::entities::FruitType::Blackberry: fruitKey = "Blackberry"; break;
        default: std::cerr << "[FACTORY ERROR] Unknown fruit type!\n"; return nullptr;
        }

        if (config.contains(fruitKey))
        {
            const auto& data = config[fruitKey];

            int hp = data.value("hp", 100);
            float speed = data.value("maxSpeed", 400.0f);
            float attackSpeed = data.value("attackSpeed", 1.0f);
            int idleFrames = data.value("idleFrames", 4);
            int walkFrames = data.value("walkFrames", 4);

            // 1. STATYSTYKI
            entity->addComponent(std::make_unique<game::components::StatsComponent>(hp, attackSpeed));

            // 2. RUCH I FIZYKA
            entity->addComponent(std::make_unique<game::components::MovementComponent>(game, speed));
            entity->addComponent(std::make_unique<game::components::ColliderComponent>(collisionMask, mapScale));

            // 3. GRAFIKA (Odczytana z czystego ResourceManagera!)
            auto& res = game::core::ResourceManager::get();
            sf::Texture* idleTex = res.hasTexture(fruitKey + "_idle") ? &res.getTexture(fruitKey + "_idle") : nullptr;
            sf::Texture* walkTex = res.hasTexture(fruitKey + "_walk") ? &res.getTexture(fruitKey + "_walk") : nullptr;
            entity->addComponent(std::make_unique<game::components::SpriteComponent>(
                idleTex, idleFrames, walkTex, walkFrames
            ));

            // 4. UMIEJ?TNO?CI
            auto abilities = std::make_unique<game::components::AbilityComponent>();
            if (data.contains("abilities"))
            {
                for (const auto& abName : data["abilities"])
                {
                    std::string abilityName = abName.get<std::string>();

                    if (abilityName == "Shoot") {
                        abilities->setWeapon(std::make_unique<game::components::ShootAbility>(*context.bullets));
                    }
                    else if (abilityName == "Shotgun") {
                        abilities->setWeapon(std::make_unique<game::components::ShotgunAbility>(*context.bullets));
                    }
                    else if (abilityName == "Dash") {
                        abilities->setSkill(std::make_unique<game::components::DashAbility>(entity.get()));
                    }
                    else if (abilityName == "AcidSquirt") {
                        std::string texPath = data.value("projectileTexture", "assets/textures/default_bullet.png");
                        abilities->setWeapon(std::make_unique<game::components::AcidSquirtAbility>(*context.bullets, entity.get(), texPath));
                    }
                    else if (abilityName == "RindRoll") {
                        float rDur = data.value("rollDuration", 1.5f);
                        float rSpd = data.value("rollSpeed", 80.0f);
                        abilities->setSkill(std::make_unique<game::components::RindRollAbility>(entity.get(), rDur, rSpd));
                    }
                }
            }
            entity->addComponent(std::move(abilities));
        }

        return entity;
    }
}