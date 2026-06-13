#pragma once
#include "State.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

namespace game::states
{
    // --- Rozbudowane klasy rzadkoœci ---
    enum class ItemRarity { Common, Epic, Mythic, Unique };

    // Struktura dla pojedynczego efektu (pozwala na trade-offs: coœ za coœ)
    struct ItemEffect {
        std::string stat;
        float value = 0.0f;
    };

    struct ShopInGameItem {
        std::string id;
        std::string name;
        std::string desc;
        int cost = 0;
        std::string iconKey;               // Klucz do tekstury ikony ulepszenia
        std::vector<ItemEffect> effects;   // TABLICA EFEKTÓW (Data-Driven)
        ItemRarity rarity = ItemRarity::Common;
    };

    struct UIInGameItem {
        sf::RectangleShape bg;
        std::optional<sf::Text> name;
        std::optional<sf::Text> desc;
        std::optional<sf::Text> cost;
        std::optional<sf::Sprite> costIcon;
        std::optional<sf::Sprite> itemIcon; // Grafika samego przedmiotu wewn¹trz karty

        // --- Nowe przyciski wewn¹trz karty ---
        sf::RectangleShape lockBtn;
        std::optional<sf::Text> lockText;

        ShopInGameItem data;
        bool soldOut = false;
        bool isLocked = false;             // Czy karta zosta³a zamro¿ona na kolejn¹ rundê

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