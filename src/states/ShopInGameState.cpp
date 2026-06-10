// --- ShopInGameState.cpp ---

#include "ShopInGameState.h"
#include "../components/StatsComponent.h"
#include "../core/ArenaContext.h"
#include "../core/Game.h"
#include "../core/LocalizationManager.h"
#include "../core/ResourceManager.h"
#include <random>
#include <algorithm>
#include <cmath>

namespace game::states
{
    using namespace game::core;

    ShopInGameState::ShopInGameState(game::Game* game)
        : State(game)
    {
        initUI();
        loadPool();
        rollItems();
    }

    void ShopInGameState::initUI()
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        // Overlay
        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(10, 15, 20, 210));

        // --- TYPOGRAFIA ---
        titleText.emplace(game->mainFont, LocUTF8("ui_in_game_shop_title"), static_cast<int>(50 * GLOBAL_FONT_SCALE));
        titleText->setFillColor(sf::Color::White);
        titleText->setOutlineColor(sf::Color::Black);
        titleText->setOutlineThickness(4.f);
        sf::FloatRect tBounds = titleText->getLocalBounds();
        titleText->setOrigin({ std::round(tBounds.size.x / 2.0f), std::round(tBounds.position.y + tBounds.size.y / 2.0f) });
        titleText->setPosition({ centerX, 60.f });

        biomassText.emplace(game->mainFont, "", static_cast<int>(26 * GLOBAL_FONT_SCALE));
        biomassText->setFillColor(sf::Color(10, 230, 255));
        biomassText->setOutlineColor(sf::Color::Black);
        biomassText->setOutlineThickness(2.f);

        auto& rm = ResourceManager::get();

        // Gorna ikona
        if (rm.hasTexture("juice")) {
            biomassIcon.emplace(*rm.getTexture("juice"));
            biomassIcon->setScale({ 1.8f, 1.8f });
            biomassIcon->setOrigin({ biomassIcon->getTexture().getSize().x / 2.f, biomassIcon->getTexture().getSize().y / 2.f });
        }

        if (!rm.hasTexture("ui_empty_button")) {
            rm.loadTexture("ui_empty_button", "assets/textures/ui/empty_button.png", AssetGroup::Global);
        }

        // --- PRZYCISK RESUME) ---
        setupButton("ui_empty_button", resumeBtnSprite, { centerX, viewSize.y - 80.f }, { 250.f, 60.f });
        setupButtonText(resumeText, LocUTF8("ui_resume"), { centerX, viewSize.y - 85.f }, 24);

        // --- PRZYCISK REROLL ---
        setupButton("ui_empty_button", rerollBtnSprite, { viewSize.x - 180.f, viewSize.y - 80.f }, { 250.f, 60.f });
        setupButtonText(rerollText, LocUTF8("ui_reroll") + " (" + std::to_string(rerollCost) + ")", { viewSize.x - 180.f, viewSize.y - 85.f }, 24);
    }

    void ShopInGameState::loadPool()
    {
        commonPool.clear();
        epicPool.clear();
        uniquePool.clear();

        if (game->upgradesConfig.contains("Common")) {
            for (const auto& up : game->upgradesConfig["Common"]) {
                commonPool.push_back({ up["id"], up["name"], up["desc"], up["cost"], up["target"], up["value"], ItemRarity::Common });
            }
        }
        if (game->upgradesConfig.contains("Epic")) {
            for (const auto& up : game->upgradesConfig["Epic"]) {
                epicPool.push_back({ up["id"], up["name"], up["desc"], up["cost"], up["target"], up["value"], ItemRarity::Epic });
            }
        }

        std::string fruitKey = game->selectedFruitKey;
        if (game->fruitsConfig.contains(fruitKey) && game->fruitsConfig[fruitKey].contains("upgrades")) {
            for (const auto& up : game->fruitsConfig[fruitKey]["upgrades"]) {
                uniquePool.push_back({ up["id"], up["name"], up["desc"], up["cost"], up["target"], up["value"], ItemRarity::Unique });
            }
        }
    }

    void ShopInGameState::rollItems()
    {
        currentDisplay.clear();
        uiSlots.clear();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> chanceDist(1, 100);

        auto tempCommon = commonPool;
        auto tempEpic = epicPool;
        auto tempUnique = uniquePool;

        for (int i = 0; i < 3; ++i) {
            int roll = chanceDist(gen);
            ShopInGameItem selectedItem;

            if (roll <= 3 && !tempUnique.empty()) {
                std::uniform_int_distribution<size_t> dist(0, tempUnique.size() - 1);
                size_t idx = dist(gen);
                selectedItem = tempUnique[idx];
                tempUnique.erase(tempUnique.begin() + idx);
            }
            else if (roll <= 13 && !tempEpic.empty()) {
                std::uniform_int_distribution<size_t> dist(0, tempEpic.size() - 1);
                size_t idx = dist(gen);
                selectedItem = tempEpic[idx];
                tempEpic.erase(tempEpic.begin() + idx);
            }
            else if (!tempCommon.empty()) {
                std::uniform_int_distribution<size_t> dist(0, tempCommon.size() - 1);
                size_t idx = dist(gen);
                selectedItem = tempCommon[idx];
                tempCommon.erase(tempCommon.begin() + idx);
            }

            currentDisplay.push_back(selectedItem);
        }

        // --- IDEALNE WYSRODKOWANIE KART ---
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        float cardWidth = 320.f;
        float cardHeight = 420.f;
        float spacingX = 40.f;

        // Obliczamy ca³kowita szerokosc zajmowana przez wszystkie 3 karty i odstepy miedzy nimi
        float totalWidth = (3 * cardWidth) + (2 * spacingX);

        // Zaczynamy od lewej krawedzi bloku, dodajac po³owe szerokosci karty, by dopasowac do origin
        float startX = centerX - (totalWidth / 2.0f) + (cardWidth / 2.0f);

        auto& rm = ResourceManager::get();

        for (int i = 0; i < currentDisplay.size(); ++i) {
            UIInGameItem ui;
            ui.data = currentDisplay[i];

            ui.bg.setSize({ cardWidth, cardHeight });
            ui.bg.setOrigin({ cardWidth / 2.0f, cardHeight / 2.0f });
            ui.bg.setPosition({ startX + (i * (cardWidth + spacingX)), centerY });
            ui.bg.setFillColor(sf::Color(40, 45, 55, 240));
            ui.bg.setOutlineThickness(4.f);

            if (ui.data.rarity == ItemRarity::Common) ui.bg.setOutlineColor(sf::Color(40, 150, 255));
            else if (ui.data.rarity == ItemRarity::Epic) ui.bg.setOutlineColor(sf::Color(180, 50, 255));
            else if (ui.data.rarity == ItemRarity::Unique) ui.bg.setOutlineColor(sf::Color(255, 215, 0));

            sf::Vector2f innerPos = ui.bg.getPosition() - sf::Vector2f(cardWidth / 2.0f, cardHeight / 2.0f);

            setupButtonText(ui.name, ui.data.name, { ui.bg.getPosition().x, innerPos.y + 40.f }, 24);
            ui.name->setFillColor(ui.bg.getOutlineColor());

            setupButtonText(ui.desc, ui.data.desc, { ui.bg.getPosition().x, innerPos.y + 200.f }, 18);
            ui.desc->setFillColor(sf::Color(200, 200, 200));


            setupButtonText(ui.cost, std::to_string(ui.data.cost), { ui.bg.getPosition().x - 15.f, innerPos.y + cardHeight - 40.f }, 24);
            ui.cost->setFillColor(sf::Color(255, 215, 0));

            if (rm.hasTexture("juice")) {
                ui.costIcon.emplace(*rm.getTexture("juice"));
                ui.costIcon->setScale({ 1.3f, 1.3f });
                ui.costIcon->setOrigin({ ui.costIcon->getTexture().getSize().x / 2.f, ui.costIcon->getTexture().getSize().y / 2.f });
                ui.costIcon->setPosition({ ui.bg.getPosition().x + 25.f, innerPos.y + cardHeight - 40.f });
            }

            uiSlots.push_back(std::move(ui));
        }
    }

    void ShopInGameState::applyUpgrade(const ShopInGameItem& item)
    {
        auto* playerStats = game->arenaContext.playerStats;
        if (!playerStats) return;

        if (item.targetStat == "hp") {
            playerStats->increaseMaxHealth(item.value);
        }
        else if (item.targetStat == "mana") {
            playerStats->increaseMaxMana(item.value);
        }
        else if (item.targetStat == "speed_pct") {
            playerStats->multiplyBaseSpeed(1.0f + item.value);
        }
        else if (item.targetStat == "dmg_pct") {
            //playerStats->addDamageMultiplier(item.value);
        }
        else if (item.targetStat == "lifesteal") {
            // playerStats->addLifesteal(item.value);
        }
        else if (item.targetStat == "ultRate") {
            playerStats->increaseUltChargeRate(item.value);
        }
        else if (item.targetStat == "bonusProjectiles") {
            playerStats->setBonusProjectiles(playerStats->getBonusProjectiles() + static_cast<int>(item.value));
        }
    }

    void ShopInGameState::handleEvent(const sf::Event& event)
    {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(mousePos, game->getWindow().getDefaultView());

                if (rerollBtnSprite && rerollBtnSprite->getGlobalBounds().contains(worldPos)) {
                    if (game->profile.biomassJuice >= rerollCost) {
                        game->playUIClick();
                        game->profile.spendJuice(rerollCost);
                        rollItems();
                    }
                }

                if (resumeBtnSprite && resumeBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().popState();
                }

                for (auto& slot : uiSlots) {
                    if (!slot.soldOut && slot.bg.getGlobalBounds().contains(worldPos)) {
                        if (game->profile.biomassJuice >= slot.data.cost) {
                            game->playUIClick();
                            game->profile.spendJuice(slot.data.cost);
                            applyUpgrade(slot.data);

                            slot.soldOut = true;
                            slot.bg.setFillColor(sf::Color(30, 30, 30, 200));
                            slot.bg.setOutlineColor(sf::Color(50, 50, 50));

                            slot.name->setString(LocUTF8("ui_sold"));
                            slot.name->setFillColor(sf::Color(100, 100, 100));

                            slot.desc->setString("");
                            slot.cost->setString("");

                            if (slot.costIcon) {
                                slot.costIcon->setColor(sf::Color(255, 255, 255, 0)); // Ukrycie ikony
                            }
                        }
                    }
                }
            }
        }

        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Space) {
                game->playUIClick();
                game->getStateMachine().popState();
            }
        }
    }

    void ShopInGameState::update(float /*dt*/)
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        float centerX = viewSize.x / 2.0f;

        // Ustawienie tekstu i ikony obok siebie
        biomassText->setString(std::to_string(game->profile.biomassJuice));
        sf::FloatRect bBounds = biomassText->getLocalBounds();
        biomassText->setOrigin({ 0.f, std::round(bBounds.position.y + bBounds.size.y / 2.0f) });

        float totalWidth = 35.f + bBounds.size.x;
        float startX = centerX - (totalWidth / 2.f);

        if (biomassIcon) biomassIcon->setPosition({ startX + 15.f, 130.f });
        biomassText->setPosition({ startX + 45.f, 130.f });

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        updateHover(rerollBtnSprite, { 250.0f, 60.0f }, mousePos, &rerollText);
        updateHover(resumeBtnSprite, { 250.0f, 60.0f }, mousePos, &resumeText);

        for (auto& slot : uiSlots) {
            if (!slot.soldOut) {
                if (slot.bg.getGlobalBounds().contains(mousePos)) {
                    slot.bg.setOutlineColor(sf::Color::Yellow);
                }
                else {
                    if (slot.data.rarity == ItemRarity::Common) slot.bg.setOutlineColor(sf::Color(40, 150, 255));
                    else if (slot.data.rarity == ItemRarity::Epic) slot.bg.setOutlineColor(sf::Color(180, 50, 255));
                    else if (slot.data.rarity == ItemRarity::Unique) slot.bg.setOutlineColor(sf::Color(255, 215, 0));
                }
            }
        }
    }

    void ShopInGameState::render(sf::RenderWindow& window)
    {
        window.setView(window.getDefaultView());
        window.draw(darkOverlay);

        if (titleText) window.draw(*titleText);

        if (biomassIcon) window.draw(*biomassIcon);
        if (biomassText) window.draw(*biomassText);

        for (const auto& slot : uiSlots) {
            window.draw(slot.bg);
            if (slot.name) window.draw(*slot.name);
            if (slot.desc) window.draw(*slot.desc);
            if (slot.cost) window.draw(*slot.cost);
            if (slot.costIcon) window.draw(*slot.costIcon);
        }

        if (rerollBtnSprite) window.draw(*rerollBtnSprite);
        if (rerollText) window.draw(*rerollText);

        if (resumeBtnSprite) window.draw(*resumeBtnSprite);
        if (resumeText) window.draw(*resumeText);

        game->drawMenuCursor();
    }
}