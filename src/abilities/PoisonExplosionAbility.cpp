#include "PoisonExplosionAbility.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"
#include "../components/TransformComponent.h"
#include "../components/AoEComponent.h"
#include "../components/LifespanComponent.h"
#include "../components/SpriteComponent.h"
#include "../core/ResourceManager.h"

namespace game::components
{
    PoisonExplosionAbility::PoisonExplosionAbility(
        game::ArenaContext* context,
        game::entities::Entity* owner,
        float radius,
        float dps,
        float duration,
        const std::string& textureKey)
        : context_(context), owner_(owner), radius_(radius), dps_(dps), duration_(duration), textureKey_(textureKey)
    {
    }

    void PoisonExplosionAbility::update(float deltaTime)
    {
        // Ta umiej?tno?? nie ma cooldownu, odpala si? raz i niszczy w?a?ciciela
    }

    void PoisonExplosionAbility::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (!context_ || !owner_ /* || owner_->isDead() */ ) return;

        auto cloudEntity = std::make_unique<game::entities::Entity>();
        if (auto* transform = cloudEntity->getComponent<TransformComponent>()) {
            transform->position = origin;
            transform->scale = { 2.0f, 2.0f }; // Skala chmury (dostosuj do PNG)
        }

        // 1. NAPRAWA SPOWOLNIENIA: 
        // 0.6f oznacza, ?e gracz w chmurze porusza si? z 60% swojej normalnej pr?dko?ci
        auto poisonAoE = std::make_unique<AoEComponent>(
            radius_, sf::Color::Transparent, dps_, false, 0.0f, true, 0.6f, false);
        // Ukrywamy domy?lne rysowanie okr?gu, bo teraz u?yjemy tekstury!
        poisonAoE->isFriendly = false;
        poisonAoE->isVisible = false;

        cloudEntity->addComponent(std::move(poisonAoE));

        // 2. DODANIE TEKSTURY (z obs?ug? przezroczysto?ci)
        if (!textureKey_.empty())
        {
            auto& rm = game::core::ResourceManager::get();
            if (rm.hasTexture(textureKey_)) {
                auto spriteComp = std::make_unique<game::components::SpriteComponent>();
                spriteComp->setTexture(rm.getTextureShared(textureKey_));
                spriteComp->setTint(sf::Color(255, 255, 255, 180));

                cloudEntity->addComponent(std::move(spriteComp));
            }
        }

        cloudEntity->addComponent(std::make_unique<LifespanComponent>(duration_, true));
        context_->spawnEntity(std::move(cloudEntity));

        // Samobójstwo Kamikaze
        owner_->destroy();
    }
}