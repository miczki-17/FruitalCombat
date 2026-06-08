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

#include "../abilities/AcidSquirtAbility.h"
#include "../abilities/ShotgunAbility.h"
#include "../abilities/DashAbility.h"
#include "../abilities/RindRollAbility.h"
#include "../abilities/ShootAbility.h"

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

        // STATS
        entity->addComponent(std::make_unique<game::components::StatsComponent>(
            dna.maxHp, 1.0f, dna.speed));

        // DNA tracking
        entity->addComponent(std::make_unique<game::components::DNAComponent>(
            dna, targetPlayer));

        // AI
        entity->addComponent(std::make_unique<game::components::AiInputComponent>(
            targetPlayer, dna.behavior, dna.speed));

        // MOVEMENT
        entity->addComponent(std::make_unique<game::components::MovementComponent>(
            nullptr, dna.speed));

        // COLLIDER
        float baseRadius = 25.0f;
        entity->addComponent(std::make_unique<game::components::ColliderComponent>(
            collisionMask, mapScale, baseRadius * dna.sizeScale));

        // Odczytanie bazowych danych z JSONa na podstawie genotypu skinKey
        const auto& baseData = enemiesConfig.value(dna.skinKey, nlohmann::json::object());

        // SPRITE - Dynamiczne wczytywanie z JSONa (z fallbackiem)
        auto& res = game::core::ResourceManager::get();

        const std::string idleKey = dna.skinKey + "_idle";
        const std::string walkKey = dna.skinKey + "_walk";

        sf::Texture* idleTex = res.hasTexture(idleKey)
            ? res.getTexture(idleKey)
            : nullptr;

        sf::Texture* walkTex = res.hasTexture(walkKey)
            ? res.getTexture(walkKey)
            : nullptr;

        const int idleFrames = baseData.value("idleFrames", 2);
        const int walkFrames = baseData.value("walkFrames", 2);

        auto spriteComp = std::make_unique<game::components::SpriteComponent>(idleTex, 4, walkTex, 4);

        // DNA definiuje kolor - nak?adamy go na bazow? tekstur?
        spriteComp->setTint(sf::Color(
            static_cast<std::uint8_t>(dna.r),
            static_cast<std::uint8_t>(dna.g),
            static_cast<std::uint8_t>(dna.b)
        ));
        spriteComp->setCustomScale(dna.sizeScale);

        // [Tutaj miejsce na podpi?cie bazowych Shaderów dla zmutowanych wariantów]

        entity->addComponent(std::move(spriteComp));

        // ABILITIES
        auto abilities = std::make_unique<game::components::AbilityComponent>();

        for (const auto& abName : dna.abilities)
        {
            if (abName == "Shoot") {
                std::string projTex = baseData.value("projectileTexturePath", "default_bullet");
                abilities->setWeapon(std::make_unique<game::components::ShootAbility>(
                    &context, entity.get(), projTex, 1.5f)); // Zwi?kszona pr?dko??/skala
            }
            else if (abName == "Armor") {
                // Armor mo?e by? pasywn? umiej?tno?ci? przypinan? do StatsComponent, 
                // ale obs?ugiwan? w systemie obra?e?.
                auto* stats = entity->getComponent<game::components::StatsComponent>();
                if (stats) stats->setDamageReduction(0.30f); // Dodaj t? metod? do StatsComponent
            }
            else if (abName == "SplitOnDeath") {
                // Dodaj nowy komponent reaguj?cy na zniszczenie
                // entity->addComponent(std::make_unique<game::components::SplitRattleComponent>(&context, 4, "Fries"));
            }
            else if (abName == "WindupBruiser") {
                // BruiserAttackAbility wewn?trz metody update zatrzymuje MovementComponent na 0.5s przed uderzeniem
                // abilities->setSkill(std::make_unique<game::components::BruiserAttackAbility>(entity.get()));
            }
            else if (abName == "PoisonExplosion") {
                // abilities->setSkill(std::make_unique<game::components::ExplosionAbility>(&context, entity.get(), 150.0f));
            }
            // ... reszta starych umiej?tno?ci ...
        }

        entity->addComponent(std::move(abilities));

        
        // MAPS DEPENDENCY CONFIGURATION

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

        // Poka? umiej?tno?ci tylko, je?li zmutowany ma jakie? specjalne geny
        if (!dna.abilities.empty())
        {
            // Pobierz g?ówn? czcionk? gry (zmie? klucz na taki, jakiego u?ywasz w grze)
			auto font = game->mainFont.getInfo().family.empty() ? nullptr : &game->mainFont;

            if (font)
            {
                // yOffset = 40.0f * dna.sizeScale (?eby tekst unosi? si? wy?ej dla wi?kszych bossów, np. Brocc-Hulk)
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