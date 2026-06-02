// --- ShopState.cpp ---

#include "ShopState.h"
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

    ShopState::ShopState(game::Game* game)
        : State(game)
    {
        initUI();
        loadPool();
        rollItems();
    }

    void ShopState::initUI()
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        // Overlay
        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(10, 15, 20, 210));

        // --- TYPOGRAFIA ---
        titleText.emplace(game->mainFont, LocUTF8("ui_shop_title"), static_cast<int>(50 * GLOBAL_FONT_SCALE));
        titleText->setFillColor(sf::Color::White);
        titleText->setOutlineColor(sf::Color::Black);
        titleText->setOutlineThickness(4.f);
        sf::FloatRect tBounds = titleText->getLocalBounds();
        titleText->setOrigin({ std::round(tBounds.size.x / 2.0f), std::round(tBounds.position.y + tBounds.size.y / 2.0f) });
        titleText->setPosition({ centerX, 60.f });

        biomassText.emplace(game->mainFont, "", static_cast<int>(24 * GLOBAL_FONT_SCALE));
        biomassText->setFillColor(sf::Color(10, 230, 255));

        // --- PRZYCISK REROLL ---
        auto& rm = ResourceManager::get();
        if (!rm.hasTexture("ui_empty_button")) {
            rm.loadTexture("ui_empty_button", "assets/textures/ui/empty_button.png", AssetGroup::Global);
        }

        setupButton("ui_empty_button", rerollBtnSprite, { centerX, viewSize.y - 80.f }, { 300.f, 60.f });
        setupButtonText(rerollText, LocUTF8("ui_reroll") + " (" + std::to_string(rerollCost) + ")", {centerX, viewSize.y - 80.f - 5.f}, 24);
    }

    void ShopState::loadPool()
    {
        // dsc zaladowac kiedys do jsonow
        allPossibleUpgrades.push_back({ "hp1", "Thick Skin", "+50 Max HP", 50, "hp", 50.0f });
        allPossibleUpgrades.push_back({ "spd1", "Light Roots", "+15% Speed", 60, "speed", 1.15f });
        allPossibleUpgrades.push_back({ "atk1", "Fast Twitch", "+10% Atk Speed", 70, "atkSpeed", 1.1f });

        std::string charKey = game->selectedMapKey;
        auto fruitType = game->selectedFruitType;
        std::string fruitName = (fruitType == entities::FruitType::Orange) ? "Orange" : "Apple";

        if (game->fruitsConfig.contains(fruitName) && game->fruitsConfig[fruitName].contains("upgrades")) {
            for (const auto& up : game->fruitsConfig[fruitName]["upgrades"]) {
                allPossibleUpgrades.push_back({
                    up["id"], up["name"], up["desc"], up["cost"], up["target"], up["value"]
                    });
            }
        }
    }

    void ShopState::rollItems()
    {
        currentDisplay.clear();
        uiSlots.clear();

        std::vector<ShopItem> tempPool = allPossibleUpgrades;
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(tempPool.begin(), tempPool.end(), g);

        int numItems = std::min(3, static_cast<int>(tempPool.size()));

        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        float centerX = viewSize.x / 2.0f;

        // Zmienne do precyzyjnego ukladania kart
        float slotWidth = 450.f;
        float slotHeight = 110.f;
        float spacingY = 20.f;
        float startY = 180.f;

        for (int i = 0; i < numItems; ++i) {
            currentDisplay.push_back(tempPool[i]);

            UIItem ui;
            ui.data = tempPool[i];

            // Tlo Karty
            ui.bg.setSize({ slotWidth, slotHeight });
            ui.bg.setOrigin({ slotWidth / 2.0f, 0.0f });
            ui.bg.setPosition({ centerX, startY + (static_cast<float>(i) * (slotHeight + spacingY)) });
            ui.bg.setFillColor(sf::Color(40, 45, 55, 240));
            ui.bg.setOutlineColor(sf::Color(100, 100, 120));
            ui.bg.setOutlineThickness(2.f);

            sf::Vector2f innerPos = ui.bg.getPosition() - sf::Vector2f(slotWidth / 2.0f, 0.f);

            setupButtonText(ui.name, ui.data.name, { innerPos.x + 20.f, innerPos.y + 30.f }, 24);
            ui.name->setOrigin({ 0.f, ui.name->getOrigin().y }); // Wyrownanie do lewej
            ui.name->setFillColor(sf::Color::White);

            setupButtonText(ui.desc, ui.data.desc, { innerPos.x + 20.f, innerPos.y + 60.f }, 16);
            ui.desc->setOrigin({ 0.f, ui.desc->getOrigin().y });
            ui.desc->setFillColor(sf::Color(180, 180, 180));

            // Koszt po prawej stronie
            setupButtonText(ui.cost, std::to_string(ui.data.cost) + " Juice", { innerPos.x + slotWidth - 20.f, innerPos.y + 30.f }, 22);
            sf::FloatRect costBounds = ui.cost->getLocalBounds();
            ui.cost->setOrigin({ costBounds.size.x, ui.cost->getOrigin().y }); // Wyrownanie do prawej
            ui.cost->setFillColor(sf::Color(255, 215, 0));

            uiSlots.push_back(std::move(ui));
        }
    }

    void ShopState::applyUpgrade(const ShopItem& item)
    {
        auto* playerStats = game->arenaContext.playerStats;
        if (!playerStats) return;

        if (item.targetStat == "hp") {
            playerStats->increaseMaxHealth(item.value);
        }
        else if (item.targetStat == "speed") {
            playerStats->multiplyBaseSpeed(item.value);
        }
        else if (item.targetStat == "atkSpeed") {
            playerStats->multiplyAttackSpeed(item.value);
        }
        else if (item.targetStat == "bonusProjectiles") {
            playerStats->setBonusProjectiles(playerStats->getBonusProjectiles() + static_cast<int>(item.value));
        }
        else if (item.targetStat == "ultRate") {
            playerStats->increaseUltChargeRate(item.value);
        }
    }

    void ShopState::handleEvent(const sf::Event& event)
    {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(mousePos, game->getWindow().getDefaultView());

                // Reroll z przyciskiem z UI
                if (rerollBtnSprite && rerollBtnSprite->getGlobalBounds().contains(worldPos)) {
                    if (game->profile.biomassJuice >= rerollCost) {
                        game->playUIClick();
                        game->profile.addJuice(-rerollCost);
                        rollItems();
                    }
                }

                // Kupowanie (karty sklepowe)
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
                        }
                    }
                }
            }
        }

        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            // Zamykanie sklepu klawiszem
            if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Space) {
                game->playUIClick();
                game->getStateMachine().popState();
            }
        }
    }

    void ShopState::update(float /*dt*/)
    {
        // Dynamiczna lokalizacja dostêpnego Juice (dodaj "ui_juice_avail" do JSON)
        std::string biomassStr = LocUTF8("ui_juice_avail") + " " + std::to_string(game->profile.biomassJuice);
        biomassText->setString(biomassStr);

        // Wysrodkowanie tekstu biomasy pod tytulem
        sf::FloatRect bBounds = biomassText->getLocalBounds();
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        biomassText->setOrigin({ std::round(bBounds.size.x / 2.0f), std::round(bBounds.position.y + bBounds.size.y / 2.0f) });
        biomassText->setPosition({ viewSize.x / 2.0f, 110.f });

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        // Hover dla przycisku Reroll -> State.h
        updateHover(rerollBtnSprite, { 300.0f, 60.0f }, mousePos, &rerollText);

        // Subtelny hover dla kart sklepowych (jesli nie wyprzedane)
        for (auto& slot : uiSlots) {
            if (!slot.soldOut) {
                if (slot.bg.getGlobalBounds().contains(mousePos)) {
                    slot.bg.setOutlineColor(sf::Color::Yellow);
                }
                else {
                    slot.bg.setOutlineColor(sf::Color(100, 100, 120));
                }
            }
        }
    }

    void ShopState::render(sf::RenderWindow& window)
    {
        window.setView(window.getDefaultView());
        window.draw(darkOverlay);

        if (titleText) window.draw(*titleText);
        if (biomassText) window.draw(*biomassText);

        for (const auto& slot : uiSlots) {
            window.draw(slot.bg);
            if (slot.name) window.draw(*slot.name);
            if (slot.desc) window.draw(*slot.desc);
            if (slot.cost) window.draw(*slot.cost);
        }

        if (rerollBtnSprite) window.draw(*rerollBtnSprite);
        if (rerollText) window.draw(*rerollText);

        game->drawMenuCursor();
    }
}