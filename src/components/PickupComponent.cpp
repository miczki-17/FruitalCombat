#include "PickupComponent.h"
#include "../entities/Entity.h"
#include "TransformComponent.h"
#include "../core/ResourceManager.h"
#include <cmath>

namespace game::components
{
    PickupComponent::PickupComponent(const std::string& textureKey, const sf::Vector2f& initialVelocity, bool useGlow)
        : velocity(initialVelocity)
    {
        auto tex = game::core::ResourceManager::get().getTextureShared(textureKey);
        if (tex)
        {
            sprite.emplace(*tex);
            sprite->setOrigin({ tex->getSize().x * 0.5f, tex->getSize().y * 0.5f });
        }

        // Je?li u?ywamy po?wiaty (np. dla monet)
        if (useGlow)
        {
            // klucz dla poswiaty
            auto glowTex = game::core::ResourceManager::get().getTextureShared(textureKey);
            if (glowTex)
            {
                glowSprite.emplace(*glowTex);
                glowSprite->setOrigin({ glowTex->getSize().x * 0.5f, glowTex->getSize().y * 0.5f });
            }
        }
    }

    void PickupComponent::update(float dt)
    {
        if (!owner) return;
        auto* transform = owner->getComponent<TransformComponent>();
        if (!transform) return;

        animationTimer += dt;
        if (spawnScale < 1.f)
        {
            spawnScale += dt * 5.f;
            if (spawnScale > 1.f) spawnScale = 1.f;
        }

        // SPAWN PHASE (Fizyka spadania po wyskoczeniu)
        if (spawning)
        {
            velocity.y += 800.f * dt; // Grawitacja
            transform->position += velocity * dt;

            if (velocity.y > 0.f) {
                spawning = false; // Przedmiot uderzy? w ziemi?
            }
        }

        // IDLE PHASE (Animacje falowania i pulsowania nad ziemi?)
        float bob = std::sin(animationTimer * 4.f) * 4.f;
        float pulse = 1.f + std::sin(animationTimer * 5.f) * 0.12f;

        sf::Vector2f renderPos = { transform->position.x, transform->position.y + bob };

        if (sprite)
        {
            sprite->setPosition(renderPos);
            sprite->setScale({ spawnScale * pulse * 1.7f, spawnScale * pulse * 1.7f });

            if (glowSprite) {
                glowSprite->setPosition(renderPos);
                glowSprite->setScale({ spawnScale * pulse * 2.2f, spawnScale * pulse * 2.2f });
            }
        }
    }

    void PickupComponent::render(sf::RenderWindow& window)
    {
        if (glowSprite) {
            sf::RenderStates additive;
            additive.blendMode = sf::BlendAdd;
            window.draw(*glowSprite, additive);
        }

        if (sprite) window.draw(*sprite);
    }
}