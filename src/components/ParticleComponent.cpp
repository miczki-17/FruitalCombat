// --- ParticleComponent.cpp ---

#include "ParticleComponent.h"
#include "../entities/Entity.h"
#include "TransformComponent.h"

namespace game::components
{
    ParticleComponent::ParticleComponent(float s, sf::Color c, float f)
        : size(s), baseColor(c), friction(f) {}

    void ParticleComponent::update(float dt)
    {
        if (!owner) return;

        if (auto* transform = owner->getComponent<TransformComponent>())
        {
            // 1. MOVE
            transform->position += transform->velocity * dt;

            // 2. FRICTION
            transform->velocity -= transform->velocity * friction * dt;
        }
    }
}