#include "MutantFactory.h"

// ECS Components
#include "../components/StatsComponent.h"
#include "../components/DNAComponent.h"
#include "../components/AiInputComponent.h"
#include "../components/ColliderComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/AbilityComponent.h"
#include "../abilities/AcidSquirtAbility.h"
#include "../abilities/ShotgunAbility.h"
#include "../abilities/DashAbility.h"
#include "../abilities/RindRollAbility.h"

#include "../core/ResourceManager.h"
#include <iostream>
#include <cstdint>

namespace game::factories
{
    MutantFactory::MutantFactory(game::ArenaContext& arenaContext, game::Game* gameRef, const sf::Image& mask, float scale)
        : context(arenaContext), game(gameRef), collisionMask(mask), mapScale(scale)
    {
    }

    std::unique_ptr<game::entities::Entity> MutantFactory::createMutant(const game::genetics::DNA& dna, game::entities::Entity* targetPlayer)
    {
        auto entity = std::make_unique<game::entities::Entity>();

        // 1. STATS: Maximum health defined by genetics
        entity->addComponent(std::make_unique<game::components::StatsComponent>(dna.maxHp, 1.0f));

        // 2. DNA TRACKER: Records fitness score during battle to guide future evolution
        entity->addComponent(std::make_unique<game::components::DNAComponent>(dna, targetPlayer));

        // 3. AI BRAIN: Movement logic governed by the genetic behavior trait and speed limit
        entity->addComponent(std::make_unique<game::components::AiInputComponent>(targetPlayer, dna.behavior, dna.speed));

        // 4. PHYSICS: Base collision radius scaled by the genetic size multiplier
        float baseRadius = 25.0f;
        entity->addComponent(std::make_unique<game::components::ColliderComponent>(collisionMask, mapScale, baseRadius * dna.sizeScale));

        // 5. VISUALS: Render shape with genetic color tint, size scaling AND INHERITED SKIN
        auto& res = game::core::ResourceManager::get();

        std::string idleKey = dna.skinKey + "_idle";
        std::string walkKey = dna.skinKey + "_walk";

        sf::Texture* idleTex = res.hasTexture(idleKey) ? &res.getTexture(idleKey) : nullptr;
        sf::Texture* walkTex = res.hasTexture(walkKey) ? &res.getTexture(walkKey) : nullptr;

        auto spriteComp = std::make_unique<game::components::SpriteComponent>(idleTex, 4, walkTex, 4);

        // SFML 3 STRICT TYPE CASTING (Zabezpieczenie przed b³êdami kompilacji)
        spriteComp->setTint(sf::Color(
            static_cast<std::uint8_t>(dna.r),
            static_cast<std::uint8_t>(dna.g),
            static_cast<std::uint8_t>(dna.b)
        ));
        spriteComp->setCustomScale(dna.sizeScale);
        entity->addComponent(std::move(spriteComp));

        // 6. ABILITIES: Dynamiczne wpinanie skilli z DNA
        auto abilities = std::make_unique<game::components::AbilityComponent>();

        for (const auto& abName : dna.abilities)
        {
            if (abName == "AcidSquirt") {
                abilities->setWeapon(std::make_unique<game::components::AcidSquirtAbility>(
                    *context.bullets, entity.get(), "assets/textures/default_bullet.png"
                    ));
            }
            else if (abName == "Shotgun") {
                // abilities->setWeapon(std::make_unique<game::components::ShotgunAbility>(*context.bullets, entity.get()));
            }
            else if (abName == "Dash") {
                abilities->setSkill(std::make_unique<game::components::DashAbility>(entity.get()));
            }
            else if (abName == "RindRoll") {
                abilities->setSkill(std::make_unique<game::components::RindRollAbility>(entity.get()));
            }
        }
        entity->addComponent(std::move(abilities));

        return entity;
    }
}