// --- AoEComponent.cpp ---

#include "AoEComponent.h"
#include "../entities/Entity.h"
#include "TransformComponent.h"

namespace game::components
{
    AoEComponent::AoEComponent(float r, sf::Color color, float damagePerSec,
        bool poison, float pDps, bool slow, float slowMult, bool friendly)
        : radius(r), dps(damagePerSec), appliesPoison(poison), poisonDps(pDps),
        appliesSlow(slow), slowMultiplier(slowMult), isFriendly(friendly)
    {
        shape.setRadius(radius);
        shape.setOrigin({ radius, radius });
        shape.setFillColor(color);
        shape.setOutlineThickness(0.0f);
    }

    void AoEComponent::setTexture(std::shared_ptr<sf::Texture> tex)
    {
        if (!tex) return;

        sprite.emplace(*tex);
        auto size = tex->getSize();
        sprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });

        float targetDiameter = radius * 2.0f;
        sprite->setScale({ targetDiameter / size.x, targetDiameter / size.y });

        sprite->setRotation( sf::degrees(static_cast<float>(std::rand() % 360)) );
    }

    void AoEComponent::update(float dt)
    {
        if (!owner) return;

        // Kszta³t strefy pod¹¿a za TransformComponent
        if (auto* transform = owner->getComponent<TransformComponent>()) {
            shape.setPosition(transform->position);

            if (sprite.has_value()) {
                sprite->setPosition(transform->position);
            }
        }
    }

    void AoEComponent::render(sf::RenderWindow& window)
    {
        if (!isVisible) return;

        if (sprite.has_value())
        {
            window.draw(*sprite);
        }
        else
        {
            window.draw(shape);
        }
    }
}