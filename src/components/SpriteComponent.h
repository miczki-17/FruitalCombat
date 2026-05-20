#pragma once
#include "Component.h"
#include "../utils/AnimationController.h"
#include "../entities/Entity.h"
#include <optional>

namespace game::components
{
    class SpriteComponent : public Component
    {
    private:
        std::optional<sf::Sprite> sprite;
        AnimationController animator;
        sf::CircleShape fallbackShape;
        bool hasTextures = false;

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
				// wyswietl biala kulke jako placeholder, jesli tekstury sie nie zaladowaly
                fallbackShape.setRadius(25.0f);
                fallbackShape.setOrigin({ 25.0f, 25.0f });
                fallbackShape.setFillColor(sf::Color::White);
            }
        }

        void update(float dt) override
        {
            if (hasTextures && sprite.has_value())
            {
                sprite->setPosition(owner->position);

                if (owner->actionTimer > 0.0f) {
                    sprite->rotate(sf::degrees(450.0f * dt));
                }
                else {
                    sprite->setRotation(sf::degrees(0.f));
                    animator.setMovementState(owner->isMoving, owner->facingRight);
                    animator.updateAndApply(*sprite, dt);
                }
            }
            else
            {
				// aktualizuj pozycje placeholdera, jesli tekstury sie nie zaladowaly
                fallbackShape.setPosition(owner->position);
            }
        }

        void render(sf::RenderWindow& window) override
        {
            if (hasTextures && sprite.has_value()) window.draw(*sprite);
            else window.draw(fallbackShape);
        }
    };
}