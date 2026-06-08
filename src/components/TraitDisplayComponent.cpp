#include "TraitDisplayComponent.h"
#include "../entities/Entity.h"
#include "TransformComponent.h"

namespace game::components
{
    // 1. ZMIANA: Inicjalizujemy sf::Text wymagan? czcionk? bezpo?rednio w li?cie inicjalizacyjnej
    TraitDisplayComponent::TraitDisplayComponent(
        const std::vector<std::string>& abilities,
        const sf::Font& font,
        float yOffset)
        : yOffset_(yOffset), traitText_(font)
    {
        traitText_.setCharacterSize(12);
        traitText_.setFillColor(sf::Color::Yellow);
        traitText_.setOutlineColor(sf::Color::Black);
        traitText_.setOutlineThickness(1.0f);

        // Z??czenie umiej?tno?ci w jeden string
        std::string displayString = "";
        if (!abilities.empty())
        {
            displayString += "[";
            for (size_t i = 0; i < abilities.size(); ++i)
            {
                displayString += abilities[i];
                if (i < abilities.size() - 1) displayString += ", ";
            }
            displayString += "]";
        }

        traitText_.setString(displayString);

        // 2. ZMIANA: Zaktualizowane API dla sf::Rect oraz setOrigin w SFML 3
        sf::FloatRect textRect = traitText_.getLocalBounds();
        traitText_.setOrigin({
            textRect.position.x + textRect.size.x / 2.0f,
            textRect.position.y + textRect.size.y / 2.0f
            });
    }

    void TraitDisplayComponent::render(sf::RenderWindow& window)
    {
        if (!owner) return;

        auto* transform = owner->getComponent<TransformComponent>();
        if (transform)
        {
            // 3. ZMIANA: Przekazujemy pozycj? jako wektor w klamrach {}, a nie dwa osobne argumenty
            traitText_.setPosition({ transform->position.x, transform->position.y - yOffset_ });
            window.draw(traitText_);
        }
    }
}