// --- MutantFactory.cpp --- 

#include "MutantFactory.h"

#include "../core/Game.h"

#include "../components/StatsComponent.h"
#include "../components/DNAComponent.h"
#include "../components/AiInputComponent.h"
#include "../components/ColliderComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/AbilityComponent.h"
#include "../components/MovementComponent.h"
#include "../components/TraitDisplayComponent.h"
#include "../components/DeathRattleComponent.h"
#include "../components/SplitOnDeathComponent.h"

#include "../abilities/AcidSquirtAbility.h"
#include "../abilities/ShotgunAbility.h"
#include "../abilities/DashAbility.h"
#include "../abilities/RindRollAbility.h"
#include "../abilities/ShootAbility.h"
#include "../abilities/PoisonExplosionAbility.h"
#include "../abilities/WindupBruisierAbility.h"

#include "../core/ResourceManager.h"

#include <cstdint>

namespace game::factories
{
    MutantFactory::MutantFactory(game::ArenaContext& arenaContext,
        game::Game* gameRef,
        const sf::Image& mask,
        float scale,
        const nlohmann::json& config)
        : context(arenaContext),
        game(gameRef),
        collisionMask(mask),
        mapScale(scale),
        enemiesConfig(config)
    {}

    std::unique_ptr<game::entities::Entity>
        MutantFactory::createMutant(const game::genetics::DNA& dna,
            game::entities::Entity* targetPlayer)
    {
        if (!targetPlayer) return nullptr;

        auto entity = std::make_unique<game::entities::Entity>();
        const auto& baseData = enemiesConfig.value(dna.skinKey, nlohmann::json::object());

        entity->addComponent(std::make_unique<game::components::StatsComponent>(
            dna.maxHp, 1.0f, dna.speed));

        entity->addComponent(std::make_unique<game::components::DNAComponent>(
            dna, targetPlayer));

        float entityAgility = baseData.value("agility", 5.0f);

        entity->addComponent(std::make_unique<game::components::AiInputComponent>(
            targetPlayer, dna.behavior, dna.speed, entityAgility));

        entity->addComponent(std::make_unique<game::components::MovementComponent>(
            nullptr, dna.speed));

        float baseRadius = 25.0f;
        entity->addComponent(std::make_unique<game::components::ColliderComponent>(
            collisionMask, mapScale, baseRadius * dna.sizeScale));

        auto& res = game::core::ResourceManager::get();

        const std::string idleKey = dna.skinKey + "_idle";
        const std::string walkKey = dna.skinKey + "_walk";

        sf::Texture* idleTex = res.hasTexture(idleKey) ? res.getTexture(idleKey) : nullptr;
        sf::Texture* walkTex = res.hasTexture(walkKey) ? res.getTexture(walkKey) : nullptr;

        const int idleFrames = baseData.value("idleFrames", 2);
        const int walkFrames = baseData.value("walkFrames", 2);

        auto spriteComp = std::make_unique<game::components::SpriteComponent>(idleTex, idleFrames, walkTex, walkFrames);

        spriteComp->setTint(sf::Color(
            static_cast<std::uint8_t>(dna.r),
            static_cast<std::uint8_t>(dna.g),
            static_cast<std::uint8_t>(dna.b)
        ));
        spriteComp->setCustomScale(dna.sizeScale);

        float baseSpriteScale = baseData.value("spriteScale", 1.0f);
        spriteComp->setCustomScale(baseSpriteScale * dna.sizeScale);
        entity->addComponent(std::move(spriteComp));

        // ABILITIES
        auto abilities = std::make_unique<game::components::AbilityComponent>();
        std::string killerName = dna.generateScientificName();

        for (const auto& abName : dna.abilities)
        {
            if (abName == "Shoot") {
                std::string projTex = baseData.contains("projectileTexturePath")
                    ? (dna.skinKey + "_bullet") : "default_bullet";

                float attackCooldown = baseData.value("attackCooldown", 2.0f);
                float projDamage = baseData.value("projectileDamage", 1.0f) * dna.damageMultiplier;
                float bulletScale = baseData.value("projectileScale", 1.2f);

                abilities->setWeapon(std::make_unique<game::components::ShootAbility>(
                    &context, entity.get(), projTex, bulletScale, attackCooldown, projDamage, killerName));
            }

            else if (abName == "Armor") {
                float damageReduction = baseData.value("damageReduction", 0.30f);
                auto* stats = entity->getComponent<game::components::StatsComponent>();
                if (stats) stats->setDamageReduction(damageReduction);
            }

            else if (abName == "SplitOnDeath") {
                int splitCount = baseData.value("splitCount", 3);
                std::string splitSkin = baseData.value("splitSkinKey", "");
                float splitScale = baseData.value("splitScale", 0.5f);

                entity->addComponent(std::make_unique<game::components::SplitOnDeathComponent>(
                    splitCount, splitSkin, splitScale));
            }

            else if (abName == "WindupBruiser")
            {
                nlohmann::json bruiserCfg = baseData.contains("bruiserConfig") ? baseData["bruiserConfig"] : nlohmann::json::object();
                abilities->setSkill(std::make_unique<game::components::WindupBruiserAbility>(
                    &context, entity.get(), targetPlayer, bruiserCfg, killerName));
            }

            else if (abName == "PoisonExplosion") {
                float explosionRadius = baseData.value("explosionRadius", 150.0f);
                float poisonDps = baseData.value("poisonDps", 5.0f) * dna.damageMultiplier;
                float cloudDuration = baseData.value("cloudDuration", 5.0f);

                std::string cloudTexKey = baseData.contains("cloudTexturePath")
                    ? (dna.skinKey + "_cloud") : "default_cloud";

                abilities->setSkill(std::make_unique<game::components::PoisonExplosionAbility>(
                    &context, entity.get(), explosionRadius, poisonDps, cloudDuration, cloudTexKey, killerName));

                entity->addComponent(std::make_unique<game::components::DeathRattleComponent>(
                    explosionRadius, poisonDps, cloudDuration, cloudTexKey));
            }
            else if (abName == "Dash") {
                abilities->setSkill(std::make_unique<game::components::DashAbility>(entity.get()));
            }
        }

        entity->addComponent(std::move(abilities));

        std::string mapKey = game->selectedMapKey;
        const auto& mapData = game->mapsConfig[mapKey];

        if (auto* moveComp = entity->getComponent<game::components::MovementComponent>()) {
            float mapFriction = mapData["physics"].value("friction", 1.0f);
            moveComp->setFriction(mapFriction);
        }

        if (auto* statsComp = entity->getComponent<game::components::StatsComponent>()) {
            float mapSpeedMulti = mapData["physics"].value("speedMultiplier", 1.0f);
            statsComp->multiplyBaseSpeed(mapSpeedMulti);
        }

        if (!dna.abilities.empty())
        {
            auto font = game->mainFont.getInfo().family.empty() ? nullptr : &game->mainFont;

            if (font)
            {
                entity->addComponent(std::make_unique<game::components::TraitDisplayComponent>(
                    dna.abilities,
                    *font,
                    40.0f * dna.sizeScale
                    ));
            }
        }

        return entity;
    }
}