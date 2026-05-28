// --- FruitFactory.cpp ---

#include "FruitFactory.h"

// Abilities
#include "../abilities/ShootAbility.h"
#include "../abilities/ShotgunAbility.h"
#include "../abilities/DashAbility.h"
#include "../abilities/AcidSquirtAbility.h"
#include "../abilities/RindRollAbility.h"
#include "../abilities/AcidPoolUltimate.h"

// Components
#include "../components/MovementComponent.h"
#include "../components/ColliderComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/AbilityComponent.h"
#include "../components/StatsComponent.h"

// Core
#include "../core/ResourceManager.h"

namespace game::factories
{
    FruitFactory::FruitFactory(
        game::ArenaContext& arenaContext,
        const nlohmann::json& jsonConfig,
        const sf::Image& mask,
        float scale,
        std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef)
        : context(arenaContext),
        config(jsonConfig),
        collisionMask(mask),
        mapScale(scale),
        enemies(enemiesRef)
    {
    }

    std::string FruitFactory::fruitTypeToString(game::entities::FruitType type)
    {
        switch (type)
        {
        case game::entities::FruitType::Apple:      return "Apple";
        case game::entities::FruitType::Banana:     return "Banana";
        case game::entities::FruitType::Orange:     return "Orange";
        case game::entities::FruitType::Cherry:     return "Cherry";
        case game::entities::FruitType::Strawberry: return "Strawberry";
        case game::entities::FruitType::Blackberry: return "Blackberry";
        }

        return "";
    }

    std::unique_ptr<game::entities::Entity> FruitFactory::createFruit(
        game::entities::FruitType type)
    {
        const std::string key = fruitTypeToString(type);

        if (!config.contains(key))
            return nullptr;

        const auto& data = config.at(key);

        auto entity = std::make_unique<game::entities::Entity>();

        // ---------------- STATS ----------------
        const int hp = data.value("hp", 100);
        const float attackSpeed = data.value("attackSpeed", 1.0f);
        const float speed = data.value("maxSpeed", 400.0f);

        entity->addComponent(
            std::make_unique<game::components::StatsComponent>(
                hp, attackSpeed));

        // ---------------- MOVEMENT ----------------
        entity->addComponent(
            std::make_unique<game::components::MovementComponent>(
                nullptr, speed));

        // ---------------- COLLIDER ----------------
        entity->addComponent(
            std::make_unique<game::components::ColliderComponent>(
                collisionMask, mapScale));

        // ---------------- SPRITE ----------------
        auto& res = game::core::ResourceManager::get();

        const std::string idleKey = key + "_idle";
        const std::string walkKey = key + "_walk";

        sf::Texture* idleTex = res.hasTexture(idleKey)
            ? res.getTexture(idleKey)
            : nullptr;

        sf::Texture* walkTex = res.hasTexture(walkKey)
            ? res.getTexture(walkKey)
            : nullptr;

        const int idleFrames = data.value("idleFrames", 4);
        const int walkFrames = data.value("walkFrames", 4);

        entity->addComponent(
            std::make_unique<game::components::SpriteComponent>(
                idleTex, idleFrames, walkTex, walkFrames));

        // ---------------- ABILITIES ----------------
        // ---------------- ABILITIES ----------------
        auto abilities = std::make_unique<game::components::AbilityComponent>();

        if (data.contains("abilities"))
        {
            for (const auto& a : data["abilities"])
            {
                // PRZEKAZUJEMY 'data' DO FUNKCJI:
                attachAbility(abilities.get(), a.get<std::string>(), entity.get(), data);
            }
        }

        entity->addComponent(std::move(abilities));

        return entity;
    }

    void FruitFactory::attachAbility(
        game::components::AbilityComponent* abilities,
        const std::string& name,
        game::entities::Entity* entity,
        const nlohmann::json& fruitData) // ODBIERAMY DANE OWOCU!
    {
        if (!abilities) return; // Safety check

        if (name == "Shoot") {
            abilities->setWeapon(std::make_unique<game::components::ShootAbility>(context.bullets));
        }
        else if (name == "Shotgun") {
            abilities->setWeapon(std::make_unique<game::components::ShotgunAbility>(context.bullets));
        }
        else if (name == "Dash") {
            abilities->setSkill(std::make_unique<game::components::DashAbility>(entity));
        }
        else if (name == "AcidSquirt") {
            // Get JSON data
            std::string texPath = fruitData.value("projectileTexturePath", "assets/textures/default_bullet.png");
            std::string splashKey = fruitData.value("splashKey", "acid_splash");

            abilities->setWeapon(std::make_unique<game::components::AcidSquirtAbility>(
                context.bullets,
                entity,
                texPath,
                splashKey,
                true
            ));
        }
        else if (name == "RindRoll") {
            const float kRadius = 130.0f;
            const float kForce = 400.0f;
            abilities->setSkill(std::make_unique<game::components::RindRollAbility>(
                entity, &context, &enemies, kRadius, kForce));
        }
        else if (name == "AcidPoolUlt") {
            abilities->setUltimate(std::make_unique<game::components::AcidPoolUltimate>(entity, &context));
        }
    }
}