// --- TextComponent.h ---

#pragma once
#include "Component.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <string>

namespace game::components
{
    class TextComponent final : public Component
    {
    public:
        sf::Text text;

        TextComponent(const sf::Font& font, const std::string& str, unsigned int size, sf::Color color);

        void update(float dt) override;
        void render(sf::RenderWindow& window);
        void setAlpha(uint8_t alpha);
    };
}