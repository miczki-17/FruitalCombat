#pragma once

#include "State.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <vector>
#include <optional>

namespace game::states
{
    class ShopState : public State
    {
    private:

        std::optional<sf::Text> titleText;
        std::optional<sf::Text> currencyText;
        std::optional<sf::Text> instructionText;

        struct UpgradeOption
        {
            sf::RectangleShape button;
            std::optional<sf::Text> text;

            int cost = 0;
            int typeId = 0;

            UpgradeOption() = default;
        };

        std::vector<UpgradeOption> options;

        void setupUpgrades();
        void applyUpgrade(int typeId);

    public:

        ShopState(game::Game* game);

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;

        StateType getType() const override;
    };
}