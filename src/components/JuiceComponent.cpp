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
    JuiceComponent::JuiceComponent(float xpValue, const sf::Vector2f& initialVelocity)
        : value(xpValue), velocity(initialVelocity)
    {
        /*static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> texDist(1, 4);
        int textureNumber = texDist(gen);

        auto tex = game::core::ResourceManager::get().getTextureShared("juice_drop_" + std::to_string(textureNumber));

        if (tex)
        {
            sprite.emplace(*tex);
            sprite->setOrigin({ tex->getSize().x * 0.5f, tex->getSize().y * 0.5f });
            glowSprite = *sprite;
        }
        else
        {
            std::cerr << "[B£¥D] JuiceComponent: Nie uda³o siê wczytaæ tekstury 'juice_drop_" << textureNumber << "'!\n";
        }*/

        auto tex = game::core::ResourceManager::get().getTextureShared("juice");

        if (tex)
        {
            sprite.emplace(*tex);
            sprite->setOrigin({ tex->getSize().x * 0.5f, tex->getSize().y * 0.5f });

            //if (value <= 1.0f) {
            //    // Zwyk³a, zielona fasolka (ma³y drop)
            //    sprite->setColor(sf::Color(100, 255, 100));
            //}
            //else if (value <= 5.0f) {
            //    // Pikantna, czerwona fasolka (œredni drop)
            //    sprite->setColor(sf::Color(255, 60, 40));
            //}
            //else {
            //    // Kosmiczna, fioletowa fasolka (du¿y drop)
            //    sprite->setColor(sf::Color(180, 50, 255));
            //}

            //glowSprite = *sprite;
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

        //if (glowSprite)
        //{
        //    glowSprite->setPosition(renderPos);
        //    glowSprite->setScale({ spawnScale * pulse * 2.0f, spawnScale * pulse * 2.0f });

        //    float alpha = 140.f + std::sin(animationTimer * 8.f) * 60.f;
        //    glowSprite->setColor(sf::Color(100, 255, 255, static_cast<std::uint8_t>(alpha)));
        //    glowSprite->rotate(sf::degrees(-dt * 20.f)); // SFML 3
        //}
    }

    void JuiceComponent::render(sf::RenderWindow& window)
    {
        sf::RenderStates additive;
        additive.blendMode = sf::BlendAdd;

        if (glowSprite) window.draw(*glowSprite, additive);
        if (sprite) window.draw(*sprite);
    }
}