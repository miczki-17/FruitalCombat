#pragma once
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace game::entities
{
    struct JuiceDrop
    {
        sf::Vector2f position;
        float value; // How much XP/Currency it yields
        sf::CircleShape shape;
        bool isCollected = false;

        JuiceDrop(sf::Vector2f pos, float xpValue) : position(pos), value(xpValue)
        {
            shape.setRadius(8.0f);
            shape.setOrigin({ 8.0f, 8.0f });
            shape.setFillColor(sf::Color(10, 230, 255, 220)); // Glowing cyan/purple light juice
            shape.setOutlineThickness(1.5f);
            shape.setOutlineColor(sf::Color::White);
            shape.setPosition(position);
        }

        void render(sf::RenderWindow& window) { window.draw(shape); }
    };
}