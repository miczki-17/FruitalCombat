#pragma once
#include "State.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

namespace game::states
{
    enum class ItemRarity { Common, Epic, Unique };

    struct ShopInGameItem {
        std::string id;
        std::string name;
        std::string desc;
        int cost = 0;
        std::string targetStat;
        float value = 0.0f;
        ItemRarity rarity = ItemRarity::Common;
    };

    struct UIInGameItem {
        sf::RectangleShape bg;
        std::optional<sf::Text> name;
        std::optional<sf::Text> desc;

        std::optional<sf::Text> cost;
        std::optional<sf::Sprite> costIcon;

        ShopInGameItem data;
        bool soldOut = false;

        UIInGameItem() = default;
    };

    class ShopInGameState : public State
    {
    private:
        sf::RectangleShape darkOverlay;

        std::optional<sf::Text> titleText;

        // Gorny licznik waluty
        std::optional<sf::Text> biomassText;
        std::optional<sf::Sprite> biomassIcon;

        // Przyciski dolne
        std::optional<sf::Sprite> rerollBtnSprite;
        std::optional<sf::Text> rerollText;
        int rerollCost = 30;

        std::optional<sf::Sprite> resumeBtnSprite;
        std::optional<sf::Text> resumeText;

        std::vector<ShopInGameItem> commonPool;
        std::vector<ShopInGameItem> epicPool;
        std::vector<ShopInGameItem> uniquePool;

        std::vector<ShopInGameItem> currentDisplay;
        std::vector<UIInGameItem> uiSlots;

        void initUI();
        void loadPool();
        void rollItems();
        void applyUpgrade(const ShopInGameItem& item);

    public:
        ShopInGameState(game::Game* game);
        ~ShopInGameState() override = default;

        StateType getType() const override { return StateType::Shop; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}