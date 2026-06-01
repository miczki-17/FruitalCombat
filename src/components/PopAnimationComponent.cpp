// --- PopAnimationComponent.cpp ---

#include "PopAnimationComponent.h"
#include "../entities/Entity.h"
#include "TransformComponent.h"
#include <algorithm>

namespace game::components
{
    void PopAnimationComponent::update(float dt)
    {
        if (!owner) return;

        currentAnimTime += dt;
        float animProgress = std::min(currentAnimTime * animSpeedMult, 1.0f);

        if (auto* transform = owner->getComponent<TransformComponent>())
        {
            transform->scale.x = startScale.x + (targetScale.x - startScale.x) * animProgress;
            transform->scale.y = startScale.y + (targetScale.y - startScale.y) * animProgress;
        }
    }
}