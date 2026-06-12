// --- TextComponent.cpp --- 

#include "TextComponent.h"
#include "../entities/Entity.h"
#include "TransformComponent.h"

namespace game::components
{
    TextComponent::TextComponent(const sf::Font& font, const std::string& str, unsigned int size, sf::Color color)
        : text(font, str, size)
    {
        text.setFillColor(color);

        // Zgodnoœæ z SFML 3: uzycie size.x / size.y zamiast width/height
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.size.x / 2.0f, bounds.size.y / 2.0f });
    }

    void TextComponent::update(float dt)
    {
        if (!owner) return;

        if (auto* transform = owner->getComponent<TransformComponent>()) {
            text.setPosition(transform->position);
        }
    }

    void TextComponent::render(sf::RenderWindow& window)
    {
        window.draw(text);
    }

    void TextComponent::setAlpha(uint8_t alpha)
    {
        sf::Color c = text.getFillColor();
        c.a = alpha;
        text.setFillColor(c);
    }
}