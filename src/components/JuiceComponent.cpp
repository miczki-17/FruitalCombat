// --- JuiceCompoennt.cpp ---

#include "JuiceComponent.h"
#include "../entities/Entity.h"
#include "TransformComponent.h"
#include "../core/ResourceManager.h"
#include <random>
#include <cmath>
#include <iostream>

namespace game::components
{
    JuiceComponent::JuiceComponent(float xpValue, const sf::Vector2f& initialVelocity, bool isCoinDrop)
        : value(xpValue), velocity(initialVelocity), isCoin(isCoinDrop)
    {
        std::string textureName = isCoin ? "coin" : "juice";
        auto tex = game::core::ResourceManager::get().getTextureShared(textureName);

        if (tex)
        {
            sprite.emplace(*tex);
            sprite->setOrigin({ tex->getSize().x * 0.5f, tex->getSize().y * 0.5f });
        }
    }

    void JuiceComponent::update(float dt)
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

        // SPAWN PHASE (Fizyka spadania po wyskoczeniu z wroga)
        if (spawning)
        {
            velocity.y += 800.f * dt; // Grawitacja soku
            transform->position += velocity * dt; // Zmieniamy pozycjê encji!

            if (velocity.y > 0.f) {
                spawning = false; // Sok uderzy³ w ziemiê
            }
        }

        // IDLE PHASE (Animacje falowania i pulsowania nad ziemi¹)
        float bob = std::sin(animationTimer * 4.f) * 4.f;
        float pulse = 1.f + std::sin(animationTimer * 5.f) * 0.12f;

        // Bierzemy prawdziw¹ pozycjê encji i dodajemy podskakiwanie tylko do renderowania
        sf::Vector2f renderPos = { transform->position.x, transform->position.y + bob };

        if (sprite)
        {
            sprite->setPosition(renderPos);
            sprite->setScale({ spawnScale * pulse * 1.7f, spawnScale * pulse * 1.7f });
            sprite->rotate(sf::degrees(dt * 0.f)); // SFML 3
        }
    }

    void JuiceComponent::render(sf::RenderWindow& window)
    {
        sf::RenderStates additive;
        additive.blendMode = sf::BlendAdd;

        if (glowSprite) window.draw(*glowSprite, additive);
        if (sprite) window.draw(*sprite);
    }
}