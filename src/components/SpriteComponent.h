#pragma once
#include "Component.h"
#include "../utils/AnimationController.h"
#include "../entities/Entity.h"
#include "StatsComponent.h"
#include <optional>

namespace game::components
{
    class SpriteComponent : public Component
    {
    private:
        std::optional<sf::Sprite> sprite;
        AnimationController animator;

        // Advanced fallback shape for when textures are missing
        sf::CircleShape fallbackShape;

        // Health bar visuals
        sf::RectangleShape hpBarBackground;
        sf::RectangleShape hpBarFill;

        sf::Color geneticColor = sf::Color::White; // Stores the true mutant color
        float hitFlashTimer = 0.0f;                // Duration of the white flash

        bool hasTextures = false;
        float currentScale = 1.0f;

    public:
        SpriteComponent(sf::Texture* idleTex, int idleFrames, sf::Texture* walkTex, int walkFrames)
        {
            if (idleTex && walkTex && idleTex->getSize().x > 0 && walkTex->getSize().x > 0)
            {
                animator.setTextures(*idleTex, idleFrames, *walkTex, walkFrames);
                sprite.emplace(animator.getDefaultTexture());
                sprite->setOrigin({ 32.0f, 32.0f });
                sprite->setScale({ 2.0f, 2.0f });
                hasTextures = true;
            }
            else
            {
                // Setup a cool-looking procedural shape
                fallbackShape.setRadius(25.0f);
                fallbackShape.setOrigin({ 25.0f, 25.0f });
                fallbackShape.setOutlineThickness(3.0f);
                fallbackShape.setOutlineColor(sf::Color(0, 0, 0, 150)); // Dark outline
            }

            // Setup health bar
            hpBarBackground.setFillColor(sf::Color(50, 50, 50, 200));
            hpBarFill.setFillColor(sf::Color(255, 50, 50, 255)); // Red health

            geneticColor = sf::Color::White;
        }

        void setTint(sf::Color color)
        {
            geneticColor = color; // Save as master color
            if (hasTextures && sprite.has_value()) sprite->setColor(color);
            else fallbackShape.setFillColor(color);
        }

        // Triggered via combat logic when taking damage
        void triggerHitFlash()
        {
            hitFlashTimer = 0.08f; // Flash white for 80 milliseconds
            if (hasTextures && sprite.has_value()) sprite->setColor(sf::Color::White);
            else fallbackShape.setFillColor(sf::Color::White);
        }

        void setCustomScale(float scaleModifier)
        {
            currentScale = scaleModifier;
            if (hasTextures && sprite.has_value()) {
                sprite->setScale({ 2.0f * scaleModifier, 2.0f * scaleModifier });
            }
            fallbackShape.setRadius(25.0f * scaleModifier);
            fallbackShape.setOrigin({ 25.0f * scaleModifier, 25.0f * scaleModifier });
        }

        void update(float dt) override
        {
            // Handle Hit Flash countdown
            if (hitFlashTimer > 0.0f) {
                hitFlashTimer -= dt;
                if (hitFlashTimer <= 0.0f) {
                    // Restore original genetic colors when flash ends
                    if (hasTextures && sprite.has_value()) sprite->setColor(geneticColor);
                    else fallbackShape.setFillColor(geneticColor);
                }
            }

            if (hasTextures && sprite.has_value())
            {
                sprite->setPosition(owner->position);
                if (owner->actionTimer > 0.0f) sprite->rotate(sf::degrees(450.0f * dt));
                else {
                    sprite->setRotation(sf::degrees(0.f));
                    animator.setMovementState(owner->isMoving, owner->facingRight);
                    animator.updateAndApply(*sprite, dt);
                }
            }
            else
            {
                fallbackShape.setPosition(owner->position);
            }

            // Update Health Bar logic
            if (auto* stats = owner->getComponent<StatsComponent>())
            {
                float barWidth = 40.0f * currentScale;
                float barHeight = 6.0f;

                hpBarBackground.setSize({ barWidth, barHeight });
                hpBarBackground.setOrigin({ barWidth / 2.0f, barHeight / 2.0f });
                hpBarBackground.setPosition({ owner->position.x, owner->position.y - (35.0f * currentScale) });

                hpBarFill.setSize({ barWidth * stats->getHpPercentage(), barHeight });
                hpBarFill.setOrigin({ barWidth / 2.0f, barHeight / 2.0f });
                hpBarFill.setPosition({ owner->position.x, owner->position.y - (35.0f * currentScale) });
            }
        }

        void render(sf::RenderWindow& window) override
        {
            if (hasTextures && sprite.has_value()) window.draw(*sprite);
            else window.draw(fallbackShape);

            // Draw Health Bar if entity is not dead
            if (owner && !owner->isDead && owner->getComponent<StatsComponent>()) {
                window.draw(hpBarBackground);
                window.draw(hpBarFill);
            }
        }
    };
}