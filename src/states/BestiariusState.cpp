// --- BestiariusState.cpp ---
#include "BestiariusState.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h"
#include <cmath>
#include <iostream>

namespace game::states
{
    using namespace game::core;

    BestiariusState::BestiariusState(game::Game* game)
        : State(game)
    {
        initEntries();
        initUI();
        updatePage();
    }

    void BestiariusState::initEntries()
    {
        // 1. Ziemniak - Tarcza
        entries.push_back({
            "Pancerny \nZiemniak",
            "Ten twardy zawodnik to\nprawdopodobnie kuzyn frytek,\nale zdecydowanie mniej\nchrupiacy. Uzywa swojej\ngruboskornosci, by zaslaniac\nslabszych.\n\nZazwyczaj milczy, chyba ze\nwpadnie do wrzatku.",
            "enemy_potato_icon"
            });

        // 2. Czosnek - Wysadzacz
        entries.push_back({
            "Wybuchowy \nCzosnek",
            "Jego zapach powala na kolana,\na wybuch robi to doslownie.\nPrawdziwy koszmar wampirow.\n\nNiestabilny emocjonalnie, lubi\nglosne i wybuchowe pozegnania.\nUwaga: nie dodawac do spaghetti!",
            "enemy_garlic_icon"
            });

        // 3. Marchewa - Snajper
        entries.push_back({
            "Snajper \nMarchewa",
            "Mowili, ze od jedzenia marchwi\npoprawia sie wzrok... i mieli racje.\nTen gosc potrafi trafic cie w oko\nz drugiego konca ogrodu.\n\nZawsze zachowuje zimna krew.\nSok z niego jest niebezpieczny.",
            "enemy_carrot_icon"
            });

        // 4. Brokuł - Skoczek
        entries.push_back({
            "Skaczacy \nBrokul",
            "Trauma z dziecinstwa powraca\npod postacia zmutowanego\ndrzewka.\n\nTrenuje parkour od kielkowania.\nZanim zdazysz powiedziec\n'zdrowa dieta', on juz laduje\nz impetem na twojej glowie.",
            "enemy_broccoli_icon"
            });

        // 5. Kukurydza - Stacjonarna
        entries.push_back({
            "Kukurydz \nCKM",
            "Zwykly chlopak z pola, ale ma\npelen magazynek. Stoi w miejscu\njak wmurowany i wypluwa z siebie\npopcorn z predkoscia karabinu\nmaszynowego.\n\nPodobno w kinie robi za przekaske.",
            "enemy_corn_icon"
            });
    }

    void BestiariusState::initUI()
    {
        auto& rm = ResourceManager::get();
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        // --- 1. Tło Książki ---
        if (rm.hasTexture("ui_bestiary_book_bg")) {
            bgSprite.emplace(*rm.getTexture("ui_bestiary_book_bg"));
            sf::Vector2u bgSize(bgSprite->getTexture().getSize());
            bgSprite->setScale({ viewSize.x / bgSize.x, viewSize.y / bgSize.y });
        }

        // --- 2. Przycisk Powrotu ---
        setupButton("ui_back", backBtnSprite, { 0.0f, 0.0f }, { 60.0f, 60.0f });
        if (backBtnSprite) {
            sf::FloatRect bounds = backBtnSprite->getGlobalBounds();
            backBtnSprite->setPosition({ 20.f + (bounds.size.x / 2.0f), 20.f + (bounds.size.y / 2.0f) });
        }

        // --- 3. Tytuł na samej górze ekranu ---
        titleText.emplace(game->mainFont);
        titleText->setString("BESTIARIUSZ");
        titleText->setCharacterSize(static_cast<int>(45 * GLOBAL_FONT_SCALE));
        titleText->setFillColor(sf::Color::White);
        titleText->setOutlineThickness(4.0f);
        titleText->setOutlineColor(sf::Color::Black);
        sf::FloatRect textBounds = titleText->getLocalBounds();
        titleText->setOrigin({ std::round(textBounds.size.x / 2.0f), std::round(textBounds.position.y + textBounds.size.y / 2.0f) });
        titleText->setPosition({ centerX, 70.f });

        // --- 4. STRZAŁKI DO PRZEWIJANIA (POD KSIĄŻKĄ) ---
        float arrowOffset = 80.f; 
        float arrowY = centerY + 280.f; 

        setupButton("ui_left_arrow", leftArrowBtn, { centerX - arrowOffset, arrowY }, { 60.f, 60.f });
        setupButton("ui_right_arrow", rightArrowBtn, { centerX + arrowOffset, arrowY }, { 60.f, 60.f });

        // --- 5. TEKSTY NA KARTKACH ---
        enemyNameText.emplace(game->mainFont);
        enemyNameText->setCharacterSize(static_cast<int>(30 * GLOBAL_FONT_SCALE));
        enemyNameText->setFillColor(sf::Color(80, 50, 20));
        enemyNameText->setPosition({ centerX + 35.f, centerY - 180.f }); 

        enemyDescText.emplace(game->mainFont);
        enemyDescText->setCharacterSize(static_cast<int>(18 * GLOBAL_FONT_SCALE));
        enemyDescText->setFillColor(sf::Color(60, 40, 20));
        enemyDescText->setPosition({ centerX + 35.f, centerY - 100.f }); 
    }

    void BestiariusState::updatePage()
    {
        if (entries.empty() || currentPage < 0 || currentPage >= entries.size()) return;

        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        auto& rm = ResourceManager::get();
        auto& currentEnemy = entries[currentPage];

        
        if (enemyNameText) {
            enemyNameText->setString(currentEnemy.name);
            sf::FloatRect bounds = enemyNameText->getLocalBounds();
            enemyNameText->setOrigin({ 0.f, std::round(bounds.position.y + bounds.size.y / 2.0f) });
        }

        // Aktualizacja opisu
        if (enemyDescText) {
            enemyDescText->setString(currentEnemy.description);
        }

        // Wyświetlanie Sprite'a wroga na LEWEJ stronie
        if (rm.hasTexture(currentEnemy.textureKey)) {
            enemySprite.emplace(*rm.getTexture(currentEnemy.textureKey));
            sf::Vector2u texSize = enemySprite->getTexture().getSize();
            enemySprite->setOrigin({ texSize.x / 2.0f, texSize.y / 2.0f });

            // Środek lewej kartki
            enemySprite->setPosition({ centerX - 230.f, centerY - 50.f });
            enemySprite->setScale({ 4.0f, 4.0f });
            enemySprite->setColor(sf::Color::White);
        }
        else {
            enemySprite.reset();
        }
    }

    void BestiariusState::handleEvent(const sf::Event& event)
    {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(
                    sf::Mouse::getPosition(game->getWindow()),
                    game->getWindow().getDefaultView());

                // Powrót do Lobby
                if (backBtnSprite && backBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().popState();
                    return;
                }

                // Kliknięcie W LEWO
                if (currentPage > 0 && leftArrowBtn && leftArrowBtn->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    currentPage--;
                    updatePage();
                    return;
                }

                // Kliknięcie W PRAWO
                if (currentPage < static_cast<int>(entries.size()) - 1 && rightArrowBtn && rightArrowBtn->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    currentPage++;
                    updatePage();
                    return;
                }
            }
        }
    }

    void BestiariusState::update(float dt)
    {
        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        // Hover dla przycisków
        updateHover(backBtnSprite, { 60.0f, 60.0f }, mousePos);

        if (currentPage > 0) updateHover(leftArrowBtn, { 60.0f, 60.0f }, mousePos);
        if (currentPage < entries.size() - 1) updateHover(rightArrowBtn, { 60.0f, 60.0f }, mousePos);

        // Pływający tytuł Bestiariusza
        if (titleText) {
            float time = animationClock.getElapsedTime().asSeconds();
            float floatOffset = std::sin(time * 2.0f) * 5.0f;
            sf::Vector2f viewSize = game->getWindow().getView().getSize();
            titleText->setPosition({ viewSize.x / 2.0f, 70.f + floatOffset });
        }
    }

    void BestiariusState::render(sf::RenderWindow& window)
    {
        if (bgSprite) window.draw(*bgSprite);
        if (titleText) window.draw(*titleText);
        if (backBtnSprite) window.draw(*backBtnSprite);

        if (enemySprite) window.draw(*enemySprite);
        if (enemyNameText) window.draw(*enemyNameText);
        if (enemyDescText) window.draw(*enemyDescText);

        if (currentPage > 0 && leftArrowBtn) {
            window.draw(*leftArrowBtn);
        }
        if (currentPage < entries.size() - 1 && rightArrowBtn) {
            window.draw(*rightArrowBtn);
        }

        game->drawMenuCursor();
    }
}