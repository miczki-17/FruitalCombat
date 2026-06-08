#pragma once

#include "Component.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace game::components
{
    class TraitDisplayComponent final : public Component
    {
    public:
        TraitDisplayComponent(
            const std::vector<std::string>& abilities,
            const sf::Font& font,
            float yOffset = 40.0f);

        void render(sf::RenderWindow& window) override;

    private:
        sf::Text traitText_;
        float yOffset_;
    };
}