// --- BestiariusState.h ---
#pragma once
#include "State.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <string>

namespace game::states
{
    // Struktura przechowująca dane o potworze do wyświetlenia
    struct BestiaryEntry {
        std::string name;
        std::string description;
        std::string textureKey; // Klucz tekstury, np. "enemy_goblin_icon"
    };

    class BestiariusState : public State
    {
    private:
        std::optional<sf::Sprite> bgSprite;
        std::optional<sf::Sprite> backBtnSprite;
        std::optional<sf::Text> titleText;

        // --- STRZAŁKI NAWIGACYJNE ---
        std::optional<sf::Sprite> leftArrowBtn;
        std::optional<sf::Sprite> rightArrowBtn;

        // --- ZAWARTOŚĆ STRONY ---
        std::optional<sf::Sprite> enemySprite;
        std::optional<sf::Text> enemyNameText;
        std::optional<sf::Text> enemyDescText;

        sf::Clock animationClock;

        // --- BAZA DANYCH BESTIARIUSZA ---
        int currentPage = 0;
        std::vector<BestiaryEntry> entries;

        void initUI();
        void initEntries(); // Inicjuje potwory i ich opisy
        void updatePage();  // Odświeża UI po zmianie strony

    public:
        BestiariusState(game::Game* game);
        ~BestiariusState() override = default;

        StateType getType() const override { return StateType::Bestiarius; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}