// --- MutantFactory.cpp --- 

#include "MutantFactory.h"

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

#include <cstdint>

namespace game::factories
{
    MutantFactory::MutantFactory(game::ArenaContext& arenaContext,
        game::Game* gameRef,
        const sf::Image& mask,
        float scale)
        : context(arenaContext),
        game(gameRef),
        collisionMask(mask),
        mapScale(scale)
    {}

    std::unique_ptr<game::entities::Entity>
        MutantFactory::createMutant(const game::genetics::DNA& dna,
            game::entities::Entity* targetPlayer)
    {
        if (!targetPlayer) return nullptr;

        auto entity = std::make_unique<game::entities::Entity>();

        // STATS
        entity->addComponent(std::make_unique<game::components::StatsComponent>(
            dna.maxHp, 1.0f));

        // DNA tracking
        entity->addComponent(std::make_unique<game::components::DNAComponent>(
            dna, targetPlayer));

        // AI
        entity->addComponent(std::make_unique<game::components::AiInputComponent>(
            targetPlayer, dna.behavior, dna.speed));

        // COLLIDER
        float baseRadius = 25.0f;
        entity->addComponent(std::make_unique<game::components::ColliderComponent>(
            collisionMask, mapScale, baseRadius * dna.sizeScale));

        // SPRITE
        auto& res = game::core::ResourceManager::get();

        std::string idleKey = dna.skinKey + "_idle";
        std::string walkKey = dna.skinKey + "_walk";

        sf::Texture* idleTex = res.hasTexture(idleKey) ? res.getTexture(idleKey) : nullptr;
        sf::Texture* walkTex = res.hasTexture(walkKey) ? res.getTexture(walkKey) : nullptr;

        auto spriteComp = std::make_unique<game::components::SpriteComponent>(
            idleTex, 4, walkTex, 4);

        spriteComp->setTint(sf::Color(
            static_cast<std::uint8_t>(dna.r),
            static_cast<std::uint8_t>(dna.g),
            static_cast<std::uint8_t>(dna.b)
        ));

        spriteComp->setCustomScale(dna.sizeScale);

        entity->addComponent(std::move(spriteComp));

        // ABILITIES
        auto abilities = std::make_unique<game::components::AbilityComponent>();

        for (const auto& abName : dna.abilities)
        {
            if (abName == "AcidSquirt")
            {
                abilities->setWeapon(std::make_unique<game::components::AcidSquirtAbility>(
                    context.bullets, entity.get(), "assets/textures/default_bullet.png"));
            }
            else if (abName == "Dash")
            {
                abilities->setSkill(std::make_unique<game::components::DashAbility>(entity.get()));
            }
            else if (abName == "RindRoll")
            {
                abilities->setSkill(std::make_unique<game::components::RindRollAbility>(
                    entity.get(), &context, targetPlayer));
            }
        }

        entity->addComponent(std::move(abilities));

        return entity;
    }
}