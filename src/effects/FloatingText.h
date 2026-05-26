// --- FloatingText.h ---

#pragma once
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <string>
#include <optional>
#include <cstdint>

namespace game::effects
{
    class FloatingText
    {
    private:
        std::optional<sf::Text> text; // U?ywamy optional!
        sf::Vector2f position;
        sf::Vector2f velocity;
        float lifetime;
        float maxLifetime;

    public:
        // Przyjmujemy referencj? do czcionki
        FloatingText(const sf::Font& font, const std::string& value, sf::Vector2f startPos, sf::Color color)
            : position(startPos), lifetime(0.6f), maxLifetime(0.6f)
        {
            text.emplace(font, value, 22); // Tworzymy text wewn?trz optionala
            text->setFillColor(color);

            sf::FloatRect bounds = text->getLocalBounds();
            text->setOrigin({ bounds.size.x / 2.0f, bounds.size.y / 2.0f });
            text->setPosition(position);

            float randomX = static_cast<float>((rand() % 100) - 50);
            velocity = { randomX, -140.0f };
        }

        void update(float dt)
        {
            lifetime -= dt;
            position += velocity * dt;
            text->setPosition(position);

            float alpha = (lifetime / maxLifetime) * 255.0f;
            sf::Color c = text->getFillColor();
            text->setFillColor(sf::Color(c.r, c.g, c.b, static_cast<std::uint8_t>(alpha)));
        }

        bool isDead() const { return lifetime <= 0.0f; }

        void render(sf::RenderWindow& window) {
            if (text.has_value()) window.draw(*text);
        }
    };
}