// --- ShopState.h ---
#pragma once

#include "State.h"
#include <vector>
#include <string>
#include <optional>
#include <SFML/Graphics.hpp>

namespace game::states
{
    struct ShopItem {
        std::string id;
        std::string name;
        std::string desc;
        int cost = 0;
        std::string targetStat;
        float value = 0.0f;
    };

    struct UIItem {
        sf::RectangleShape bg;
        std::optional<sf::Text> name;
        std::optional<sf::Text> desc;
        std::optional<sf::Text> cost;
        std::optional<sf::Sprite> iconSprite; 
        ShopItem data;
        bool soldOut = false;

        UIItem() = default;
    };

    class ShopState : public State
    {
    private:
        sf::RectangleShape darkOverlay;

        std::optional<sf::Text> titleText;

        std::optional<sf::Sprite> backBtnSprite; 
        std::vector<ShopItem> allPossibleUpgrades;
        std::vector<ShopItem> currentDisplay;
        std::vector<UIItem> uiSlots;

        // Coins
        std::optional<sf::Text> coinText;
        std::optional<sf::Sprite> coinSprite;

        void initUI();
        void loadPool();
        void rollItems();
        void applyUpgrade(const ShopItem& item);

    public:
        ShopState(game::Game* game);
        ~ShopState() override = default;

        StateType getType() const override { return StateType::Shop; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}