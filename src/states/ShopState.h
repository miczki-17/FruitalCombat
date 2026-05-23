#pragma once

#include "State.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <vector>
#include <optional>
#include <string>

namespace game::states
{
    class ShopState : public State
    {
    private:
        // --- TOP BAR ---
        std::optional<sf::Text> titleText;
        std::optional<sf::Text> currencyText;
        sf::CircleShape currencyIcon; // Ma³a ¿ó³ta/zielona kulka obok tekstu

        // --- STATS PANEL (Right side) ---
        sf::RectangleShape statsPanelBg;
        std::optional<sf::Text> statsTitleText;
        std::vector<std::optional<sf::Text>> statsLines;

        // --- BOTTOM BAR ---
        sf::RectangleShape nextWaveBtn;
        std::optional<sf::Text> nextWaveText;

        // --- UPGRADE CARDS ---
        struct UpgradeOption
        {
            sf::RectangleShape cardBg;
            sf::RectangleShape buyBtnBg;

            std::optional<sf::Text> titleText;
            std::optional<sf::Text> descText;
            std::optional<sf::Text> costText;

            int cost = 0;
            int typeId = 0;
            bool isPurchased = false;

            UpgradeOption() = default;
        };

        std::vector<UpgradeOption> options;

        void setupUpgrades();
        void setupStatsPanel();
        void applyUpgrade(int typeId);
        void updateCurrencyDisplay();

    public:
        ShopState(game::Game* game);

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;

        StateType getType() const override;
    };
}