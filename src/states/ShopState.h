#pragma once
#include "State.h"
#include <vector>
#include <string>
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

    // ZMIANA: Dodano konstruktor, aby usatysfakcjonowaæ rygorystyczne wymogi SFML 3!
    struct UIItem {
        sf::RectangleShape bg;
        sf::Text name;
        sf::Text desc;
        sf::Text cost;
        ShopItem data;
        bool soldOut = false;

        // W SFML 3 musimy od razu wstrzykn¹æ czcionkê:
        UIItem(const sf::Font& font) : name(font), desc(font), cost(font) {}
    };

    class ShopState : public State
    {
    private:
        sf::Text titleText;
        sf::Text biomassText;
        sf::Text rerollText;

        sf::RectangleShape rerollButton;
        int rerollCost = 30;

        std::vector<ShopItem> allPossibleUpgrades;
        std::vector<ShopItem> currentDisplay;
        std::vector<UIItem> uiSlots;

        void loadPool();
        void rollItems();
        void applyUpgrade(const ShopItem& item);

    public:
        ShopState(game::Game* game);
        StateType getType() const override;
        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}