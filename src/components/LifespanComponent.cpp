// --- LifespanComponent.cpp ---

#include "LifespanComponent.h"
#include "../entities/Entity.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include <algorithm>

namespace game::components
{
    LifespanComponent::LifespanComponent(float duration, bool fade)
        : lifetime(duration), maxLifetime(duration), fadeOut(fade) {}

    void LifespanComponent::update(float dt)
    {
        lifetime -= dt;

        if (fadeOut && owner && maxLifetime > 0.0f)
        {
            float progress = std::clamp(lifetime / maxLifetime, 0.0f, 1.0f);
            uint8_t alpha = static_cast<uint8_t>(255.0f * progress);

            if (auto* textComp = owner->getComponent<TextComponent>()) {
                textComp->setAlpha(alpha);
            }

            // Wymaga dopisania metody setAlpha do SpriteComponent (Krok 0)
            if (auto* spriteComp = owner->getComponent<SpriteComponent>()) {
                spriteComp->setAlpha(alpha);
            }
        }

        if (lifetime <= 0.0f && owner) {
            owner->destroy();
        }
    }
}