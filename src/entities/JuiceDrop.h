#include <optional>
#include <cmath>
#include <random>
#include <iostream>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "../core/ResourceManager.h"

namespace game::entities
{
    struct JuiceDrop
    {
        sf::Vector2f position;
        float value;
        bool isCollected = false;

        std::optional<sf::Sprite> sprite;
        std::optional<sf::Sprite> glowSprite;

        float animationTimer = 0.f;

        // SPAWN ANIMATION
        sf::Vector2f velocity;
        float spawnScale = 0.75f;
        bool spawning = true;

        JuiceDrop(sf::Vector2f pos, float xpValue)
            : position(pos)
            , value(xpValue)
        {
            static std::random_device rd;
            static std::mt19937 gen(rd());

            std::uniform_int_distribution<int> texDist(1, 4);
            int textureNumber = texDist(gen);

            auto tex = game::core::ResourceManager::get().getTextureShared(
                "juice_drop_" + std::to_string(textureNumber)
            );

            if (tex)
            {
                sprite.emplace(*tex);
                sprite->setOrigin({
                    tex->getSize().x * 0.5f,
                    tex->getSize().y * 0.5f
                    });

                glowSprite = *sprite;
            }
            else
            {
                std::cerr << "[B£¥D] JuiceDrop: Nie uda³o siê wczytaæ tekstury 'juice_drop_"
                    << textureNumber << "'!\n";
            }

            // random drop
            std::uniform_real_distribution<float> angleDist(0.f, 6.283185f);
            std::uniform_real_distribution<float> speedDist(80.f, 180.f);

            float angle = angleDist(gen);
            float speed = speedDist(gen);

            velocity = {
                std::cos(angle) * speed,
                std::sin(angle) * speed - 220.f
            };

            update(0.f);
        }

        void update(float dt)
        {
            animationTimer += dt;
            if (spawnScale < 1.f)
            {
                spawnScale += dt * 5.f;
                if (spawnScale > 1.f)
                {
                    spawnScale = 1.f;
                }
            }

            // SPAWN PHASE (Fizyka spadania)
            if (spawning)
            {
                velocity.y += 800.f * dt;
                position += velocity * dt;

                if (velocity.y > 0.f)
                {
                    spawning = false;
                }
            }

            // IDLE PHASE (Animacje falowania i pulsowania)
            float bob = std::sin(animationTimer * 4.f) * 4.f;
            float pulse = 1.f + std::sin(animationTimer * 6.f) * 0.12f;

            sf::Vector2f renderPos = {
                position.x,
                position.y + bob
            };

            if (sprite)
            {
                sprite->setPosition(renderPos);
                sprite->setScale({
                    spawnScale * pulse * 2.f,
                    spawnScale * pulse * 2.f
                    });
                sprite->rotate(sf::degrees(dt * 40.f));
            }

            if (glowSprite)
            {
                glowSprite->setPosition(renderPos);
                glowSprite->setScale({
                    spawnScale * pulse * 3.f,
                    spawnScale * pulse * 3.f
                    });

                float alpha = 140.f + std::sin(animationTimer * 8.f) * 60.f;

                glowSprite->setColor(
                    sf::Color(
                        100,
                        255,
                        255,
                        static_cast<std::uint8_t>(alpha)
                    )
                );

                glowSprite->rotate(sf::degrees(-dt * 20.f));
            }
        }

        void render(sf::RenderWindow& window)
        {
            sf::RenderStates additive;
            additive.blendMode = sf::BlendAdd;

            if (glowSprite)
                window.draw(*glowSprite, additive);

            if (sprite)
                window.draw(*sprite);
        }
    };
}