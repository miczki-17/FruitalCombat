// --- LifespanComponent.cpp ---

#include "LifespanComponent.h"
#include "../entities/Entity.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "ParticleComponent.h"
#include "AoEComponent.h"
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

            // wyciemnianie czasteczek -> zmiana kanalu alpha
            if (auto* particleComp = owner->getComponent<ParticleComponent>()) {
                particleComp->baseColor.a = alpha;
            }

            // wyciemnianie stref AoE
            if (auto* aoeComp = owner->getComponent<game::components::AoEComponent>()) {
                if (aoeComp->sprite.has_value()) {
                    sf::Color spriteColor = aoeComp->sprite->getColor();
                    spriteColor.a = alpha;
                    aoeComp->sprite->setColor(spriteColor);
                }
                else
                {
                    // Wyciemnianie zapasowego ko³a
                    sf::Color c = aoeComp->shape.getFillColor();
                    c.a = alpha;
                    aoeComp->shape.setFillColor(c);
                }
            }
        }

        if (lifetime <= 0.0f && owner) {
            owner->destroy();
        }
    }
}