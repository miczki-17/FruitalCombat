#include "ShopState.h"
#include "../core/Game.h"
// Upewnij siê, ¿e masz tu odpowiednie includy do EvolutionManagera lub podawania numeru fali, jeœli przechowujesz go w Game.
#include <iostream>

namespace game::states
{
    ShopState::ShopState(game::Game* game)
        : State(game)
    {
        std::cout << "[SHOP STATE] Opened Brotato-style upgrade deck.\n";

        // --- TOP BAR SETUP ---
        titleText.emplace(game->mainFont);
        titleText->setString("Shop");
        titleText->setCharacterSize(32);
        titleText->setPosition({ 40.f, 30.f });
        titleText->setFillColor(sf::Color::White);

        currencyIcon.setRadius(12.f);
        currencyIcon.setFillColor(sf::Color(10, 230, 255)); // Twój cyjanowy kolor biomasy
        currencyIcon.setOutlineThickness(2.f);
        currencyIcon.setOutlineColor(sf::Color::White);
        currencyIcon.setPosition({ 500.f, 36.f });

        currencyText.emplace(game->mainFont);
        currencyText->setCharacterSize(28);
        currencyText->setFillColor(sf::Color::White);
        currencyText->setPosition({ 540.f, 32.f });
        updateCurrencyDisplay();

        // --- NEXT WAVE BUTTON SETUP ---
        nextWaveBtn.setSize({ 220.f, 60.f });
        // Pozycjonowanie w prawym dolnym rogu (zak³adaj¹c ekran ~1280x720. Dopasuj jeœli masz inny)
        nextWaveBtn.setPosition({ 1020.f, 620.f });
        nextWaveBtn.setFillColor(sf::Color(20, 20, 30));
        nextWaveBtn.setOutlineThickness(2.f);
        nextWaveBtn.setOutlineColor(sf::Color::White);

        nextWaveText.emplace(game->mainFont);
        nextWaveText->setString("Go (Next Wave)");
        nextWaveText->setCharacterSize(20);
        nextWaveText->setFillColor(sf::Color::White);

        // Centrowanie tekstu na przycisku
        sf::FloatRect bounds = nextWaveText->getLocalBounds();
        nextWaveText->setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });
        nextWaveText->setPosition({ nextWaveBtn.getPosition().x + 110.f, nextWaveBtn.getPosition().y + 30.f });

        setupStatsPanel();
        setupUpgrades();
    }

    void ShopState::setupStatsPanel()
    {
        // Prawy panel informacyjny
        statsPanelBg.setSize({ 260.f, 500.f });
        statsPanelBg.setPosition({ 980.f, 100.f });
        statsPanelBg.setFillColor(sf::Color(35, 35, 40)); // Lekko jaœniejszy ni¿ t³o
        statsPanelBg.setOutlineThickness(2.f);
        statsPanelBg.setOutlineColor(sf::Color(60, 60, 70));

        statsTitleText.emplace(game->mainFont);
        statsTitleText->setString("Stats");
        statsTitleText->setCharacterSize(24);
        statsTitleText->setFillColor(sf::Color::White);
        statsTitleText->setPosition({ 1070.f, 120.f });

        // Przyk³adowe statystyki do wyœwietlenia
        std::vector<std::string> statStrings = {
            "Max HP: 100",
            "Speed: 200",
            "Damage: 15.0",
            "Atk Speed: 1.0x"
        };

        float startY = 180.f;
        for (const auto& str : statStrings)
        {
            std::optional<sf::Text> line;
            line.emplace(game->mainFont);
            line->setString(str);
            line->setCharacterSize(16);
            line->setFillColor(sf::Color(200, 200, 200));
            line->setPosition({ 1000.f, startY });
            statsLines.push_back(std::move(line));
            startY += 30.f;
        }
    }

    void ShopState::setupUpgrades()
    {
        // Baza danych ulepszeñ: { Tytu³, Opis (Statystyka), Cena, Typ, Kolor Rzadkoœci }
        struct RawUpgrade {
            std::string title;
            std::string desc;
            int cost;
            int typeId;
            sf::Color rarityColor;
        };

        std::vector<RawUpgrade> upgradeData = {
            { "Cardio", "+5% Movement Speed\n\nRun faster to avoid\nmutant vegetables.", 40, 1, sf::Color(150, 150, 150) }, // Common (Szary)
            { "Vitamins", "+15 Max HP\nHeals 15 HP\n\nStay healthy and juicy.", 50, 2, sf::Color(50, 150, 255) },       // Rare (Niebieski)
            { "Frenzy", "+6% Attack Speed\n\nShoot your pips\nfaster than ever.", 60, 3, sf::Color(180, 50, 255) }       // Epic (Fioletowy)
        };

        // Unikalne dla owocu
        switch (game->selectedFruitType)
        {
        case game::entities::FruitType::Orange:
            upgradeData.push_back({ "Burst Spread", "+1 Pellet to Shotgun\n-5% Accuracy\n\nDevastating close range.", 80, 4, sf::Color(255, 50, 50) }); // Legendary (Czerwony)
            break;
        default:
            upgradeData.push_back({ "Adrenaline", "-0.15s Dash Cooldown\n\nDodge more often.", 70, 4, sf::Color(255, 200, 50) });
            break;
        }

        float startX = 40.f;
        float startY = 150.f;
        float cardWidth = 220.f;
        float cardHeight = 350.f;
        float spacing = 20.f;

        for (size_t i = 0; i < upgradeData.size(); ++i)
        {
            UpgradeOption opt;
            opt.typeId = upgradeData[i].typeId;
            opt.cost = upgradeData[i].cost;

            // Karta (T³o)
            opt.cardBg.setSize({ cardWidth, cardHeight });
            opt.cardBg.setPosition({ startX + (i * (cardWidth + spacing)), startY });
            opt.cardBg.setFillColor(sf::Color(25, 25, 30)); // Ciemnografitowy
            opt.cardBg.setOutlineThickness(2.f);
            opt.cardBg.setOutlineColor(upgradeData[i].rarityColor); // Kolor zale¿ny od rzadkoœci

            // Tytu³
            opt.titleText.emplace(game->mainFont);
            opt.titleText->setString(upgradeData[i].title);
            opt.titleText->setCharacterSize(20);
            opt.titleText->setFillColor(upgradeData[i].rarityColor);
            opt.titleText->setPosition({ opt.cardBg.getPosition().x + 15.f, opt.cardBg.getPosition().y + 15.f });

            // Opis statystyk
            opt.descText.emplace(game->mainFont);
            opt.descText->setString(upgradeData[i].desc);
            opt.descText->setCharacterSize(15);
            opt.descText->setFillColor(sf::Color(100, 255, 100)); // Zielony (pozytywne statystyki)
            opt.descText->setPosition({ opt.cardBg.getPosition().x + 15.f, opt.cardBg.getPosition().y + 60.f });

            // Przycisk Kupna na dole karty
            opt.buyBtnBg.setSize({ cardWidth - 30.f, 40.f });
            opt.buyBtnBg.setPosition({ opt.cardBg.getPosition().x + 15.f, opt.cardBg.getPosition().y + cardHeight - 55.f });
            opt.buyBtnBg.setFillColor(sf::Color(200, 200, 200));

            // Tekst ceny w przycisku
            opt.costText.emplace(game->mainFont);
            opt.costText->setString(std::to_string(opt.cost) + " Juice");
            opt.costText->setCharacterSize(16);
            opt.costText->setFillColor(sf::Color::Black);

            sf::FloatRect costBounds = opt.costText->getLocalBounds();
            opt.costText->setOrigin({ costBounds.position.x + costBounds.size.x / 2.0f, costBounds.position.y + costBounds.size.y / 2.0f });
            opt.costText->setPosition({ opt.buyBtnBg.getPosition().x + (cardWidth - 30.f) / 2.0f, opt.buyBtnBg.getPosition().y + 20.f });

            options.push_back(std::move(opt));
        }
    }

    void ShopState::updateCurrencyDisplay()
    {
        if (currencyText.has_value()) {
            currencyText->setString(std::to_string(game->playerJuice));
        }
    }

    void ShopState::applyUpgrade(int typeId)
    {
        switch (typeId)
        {
        case 1:
            // game->upgrade_speedMod += 0.05f;
            std::cout << "Speed upgraded!\n";
            break;
        case 2:
            // game->upgrade_maxHpBonus += 15;
            std::cout << "HP upgraded!\n";
            break;
        case 3:
            // game->upgrade_atkSpeedMod += 0.06f;
            std::cout << "Attack speed upgraded!\n";
            break;
        case 4:
            // game->upgrade_uniqueCount++;
            std::cout << "Unique skill upgraded!\n";
            break;
        }
    }

    void ShopState::handleEvent(const sf::Event& event)
    {
        if (auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f mouseWorld = static_cast<sf::Vector2f>(mousePos);

                // Sprawdzanie przycisku NEXT WAVE
                if (nextWaveBtn.getGlobalBounds().contains(mouseWorld))
                {
                    game->getWindow().setMouseCursorVisible(false);
                    game->getStateMachine().popState();
                    return; // Zakoñcz funkcjê, by nie sprawdzaæ kart
                }

                // Sprawdzanie kart ulepszeñ
                for (auto& opt : options)
                {
                    if (opt.isPurchased) continue; // Pomiñ kupione

                    // Mo¿esz klikn¹æ w ca³¹ kartê lub tylko w przycisk kupna
                    if (opt.cardBg.getGlobalBounds().contains(mouseWorld))
                    {
                        if (game->playerJuice >= opt.cost)
                        {
                            game->playerJuice -= opt.cost;
                            applyUpgrade(opt.typeId);
                            updateCurrencyDisplay();

                            // Oznaczenie wizualne kupienia karty
                            opt.isPurchased = true;
                            opt.cardBg.setFillColor(sf::Color(15, 15, 20)); // Wygaszenie t³a
                            opt.cardBg.setOutlineColor(sf::Color(40, 40, 40));
                            opt.buyBtnBg.setFillColor(sf::Color(40, 40, 40));
                            opt.costText->setString("PURCHASED");
                            opt.costText->setFillColor(sf::Color(100, 100, 100));

                            // Wyœrodkowanie nowego tekstu na przycisku
                            sf::FloatRect cb = opt.costText->getLocalBounds();
                            opt.costText->setOrigin({ cb.position.x + cb.size.x / 2.0f, cb.position.y + cb.size.y / 2.0f });
                            opt.costText->setPosition({ opt.buyBtnBg.getPosition().x + opt.buyBtnBg.getSize().x / 2.0f, opt.buyBtnBg.getPosition().y + 20.f });
                        }
                    }
                }
            }
        }
    }

    void ShopState::update(float dt)
    {
        // Hover effects (Podœwietlanie kart i przycisków myszk¹)
        sf::Vector2i mousePos = sf::Mouse::getPosition(game->getWindow());
        sf::Vector2f mouseWorld = static_cast<sf::Vector2f>(mousePos);

        for (auto& opt : options)
        {
            if (opt.isPurchased) continue;

            if (opt.cardBg.getGlobalBounds().contains(mouseWorld)) {
                opt.cardBg.setFillColor(sf::Color(35, 35, 45)); // Jaœniejsze t³o przy najechaniu
                opt.buyBtnBg.setFillColor(sf::Color::White);    // Bia³y przycisk
            }
            else {
                opt.cardBg.setFillColor(sf::Color(25, 25, 30)); // Standardowe
                opt.buyBtnBg.setFillColor(sf::Color(200, 200, 200));
            }
        }

        // Hover dla Next Wave
        if (nextWaveBtn.getGlobalBounds().contains(mouseWorld)) {
            nextWaveBtn.setFillColor(sf::Color(40, 40, 50));
        }
        else {
            nextWaveBtn.setFillColor(sf::Color(20, 20, 30));
        }
    }

    void ShopState::render(sf::RenderWindow& window)
    {
        // Bardzo ciemne t³o (prawie czarne, lekkie wejœcie w granat)
        window.clear(sf::Color(15, 15, 18));

        // Top bar
        if (titleText.has_value()) window.draw(*titleText);
        window.draw(currencyIcon);
        if (currencyText.has_value()) window.draw(*currencyText);

        // Stats Panel
        window.draw(statsPanelBg);
        if (statsTitleText.has_value()) window.draw(*statsTitleText);
        for (auto& line : statsLines) {
            if (line.has_value()) window.draw(*line);
        }

        // Next Wave Button
        window.draw(nextWaveBtn);
        if (nextWaveText.has_value()) window.draw(*nextWaveText);

        // Upgrade Cards
        for (auto& opt : options)
        {
            window.draw(opt.cardBg);
            window.draw(opt.buyBtnBg);

            if (opt.titleText.has_value()) window.draw(*opt.titleText);
            if (opt.descText.has_value()) window.draw(*opt.descText);
            if (opt.costText.has_value()) window.draw(*opt.costText);
        }

        game->drawMenuCursor();
    }

    game::states::StateType ShopState::getType() const
    {
        return StateType::Shop;
    }
}