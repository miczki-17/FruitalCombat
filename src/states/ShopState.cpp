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
        auto& rm = game::core::ResourceManager::get();

        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        float centerX = viewSize.x / 2.0f;

        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(10, 15, 20, 210));

        titleText.emplace(game->mainFont, "", static_cast<int>(50 * GLOBAL_FONT_SCALE));
        titleText->setString(LocUTF8("ui_shop_title"));
        titleText->setFillColor(sf::Color::White);
        titleText->setOutlineColor(sf::Color::Black);
        titleText->setOutlineThickness(4.f);
        sf::FloatRect tBounds = titleText->getLocalBounds();
        titleText->setOrigin({ std::round(tBounds.size.x / 2.0f), std::round(tBounds.position.y + tBounds.size.y / 2.0f) });
        titleText->setPosition({ centerX, 60.f });

        setupButton("ui_back", backBtnSprite, { 60.f, 60.f }, { 60.f, 60.f });
        if (backBtnSprite) {
            backBtnSprite->setPosition({ 50.f, 50.f });
        }

        if (!rm.hasTexture("coin")) {
            rm.loadTexture("coin", "assets/textures/entities/drops/juice_coin.png", game::core::AssetGroup::Global);
        }

        if (rm.hasTexture("coin"))
        {
            coinSprite.emplace(*rm.getTexture("coin"));
            coinSprite->setPosition({ centerX - 45.0f, 95.0f });
            coinSprite->setScale({ 1.8f, 1.8f });
        }
        else {
            std::cerr << "can not load coin texzture\n";
        }


        coinText.emplace(game->mainFont, "", static_cast<int>(24 * GLOBAL_FONT_SCALE));
        coinText->setFillColor(sf::Color(255, 215, 0)); 
        coinText->setPosition({ centerX, 120.f });
    }

    void ShopState::loadPool()
    {
        allPossibleUpgrades.clear();

        // 1. Ladowanie ze zwyklego configu sklepu
        if (game->shopConfig.contains("items"))
        {
            for (const auto& itemData : game->shopConfig["items"])
            {
                ShopItem item;
                item.id = itemData.value("id", "unknown");

                // TLUMACZENIE KLUCZY NA UTF-8:
                std::string nameKey = itemData.value("name", "ui_unknown");
                std::string descKey = itemData.value("desc", "");

                item.name = itemData.value("name", "ui_unknown");
                item.desc = itemData.value("desc", "");
                item.desc = descKey.empty() ? "" : LocUTF8(descKey);

                item.cost = itemData.value("cost", 999);
                item.targetStat = itemData.value("targetStat", "");
                item.value = itemData.value("value", 0.0f);

                allPossibleUpgrades.push_back(item);
            }
        }

        std::string fruitName = "Apple";
        switch (game->selectedFruitType) {
        case game::entities::FruitType::Orange: fruitName = "Orange"; break;
        case game::entities::FruitType::Banana: fruitName = "Banana"; break;
        case game::entities::FruitType::Cherry: fruitName = "Cherry"; break;
        case game::entities::FruitType::Strawberry: fruitName = "Strawberry"; break;
        case game::entities::FruitType::Blackberry: fruitName = "Blackberry"; break;
        default: fruitName = "Apple"; break;
        }

        // 2. Ladowanie z configu owocow
        if (game->fruitsConfig.contains(fruitName) && game->fruitsConfig[fruitName].contains("upgrades"))
        {
            for (const auto& up : game->fruitsConfig[fruitName]["upgrades"])
            {
                ShopItem item;
                item.id = up.value("id", "unknown");

                // TLUMACZENIE KLUCZY NA UTF-8:
                std::string nameKey = up.value("name", "ui_unknown");
                std::string descKey = up.value("desc", "");

                item.name = up.value("name", "ui_unknown");
                item.desc = up.value("desc", "");
                item.desc = descKey.empty() ? "" : LocUTF8(descKey);

                item.cost = up.value("cost", 99);
                item.targetStat = up.value("targetStat", "");
                item.value = up.value("value", 0.0f);

                allPossibleUpgrades.push_back(item);
            }
        }
    }

    void ShopState::rollItems()
    {
        uiSlots.clear();
        if (allPossibleUpgrades.empty()) return;

        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        auto& rm = game::core::ResourceManager::get();

        float cardWidth = 280.0f;
        float cardHeight = 360.0f;
        float spacing = 40.0f;
        int itemsCount = std::min(3, static_cast<int>(allPossibleUpgrades.size()));
        float totalWidth = (itemsCount * cardWidth) + ((itemsCount - 1) * spacing);
        float startX = (viewSize.x - totalWidth) / 2.0f + (cardWidth / 2.0f);
        float startY = viewSize.y / 2.0f + 40.f;

        for (int i = 0; i < itemsCount; ++i)
        {
            UIItem slot;
            slot.data = allPossibleUpgrades[i];

            slot.bg.setSize({ cardWidth, cardHeight });
            slot.bg.setOrigin({ cardWidth / 2.0f, cardHeight / 2.0f });
            slot.bg.setPosition({ startX + i * (cardWidth + spacing), startY });
            slot.bg.setFillColor(sf::Color(45, 45, 55, 240));
            slot.bg.setOutlineColor(sf::Color(100, 100, 120));
            slot.bg.setOutlineThickness(4.0f);

            if (rm.hasTexture("ui_" + slot.data.id)) {
                slot.iconSprite.emplace(*rm.getTexture("ui_" + slot.data.id));
                sf::Vector2u texSize = slot.iconSprite->getTexture().getSize();
                slot.iconSprite->setOrigin({ texSize.x / 2.0f, texSize.y / 2.0f });
                slot.iconSprite->setPosition({ slot.bg.getPosition().x, startY - 80.0f });
                slot.iconSprite->setScale({ 2.0f, 2.0f });
            }

            // NAZWA
            slot.name.emplace(game->mainFont, "", 26);
            slot.name->setString(LocUTF8(slot.data.name));
            slot.name->setOrigin({ slot.name->getLocalBounds().size.x / 2.0f, slot.name->getLocalBounds().size.y / 2.0f });
            slot.name->setPosition({ slot.bg.getPosition().x, startY + 40.0f });

            // OPIS
            auto bounds = slot.bg.getGlobalBounds();

            slot.desc.emplace(game->mainFont, "", 18);
            if (!slot.data.desc.empty()) {
                slot.desc->setString(slot.data.desc);

                auto bounds = slot.desc->getLocalBounds();

                slot.desc->setOrigin({
                    bounds.position.x + bounds.size.x / 2.f,
                    bounds.position.y + bounds.size.y / 2.f
                    });

                slot.desc->setPosition({
                    slot.bg.getPosition().x,
                    slot.bg.getPosition().y + 90.f
                    });
            }
            slot.desc->setFillColor(sf::Color(180, 180, 180));

            // KOSZT
            slot.cost.emplace(game->mainFont, std::to_string(slot.data.cost) + " Coins", 24);
            slot.cost->setFillColor(sf::Color::Yellow);
            slot.cost->setOrigin({ slot.cost->getLocalBounds().size.x / 2.0f, slot.cost->getLocalBounds().size.y / 2.0f });
            slot.cost->setPosition({ slot.bg.getPosition().x, startY + 140.0f });

            uiSlots.push_back(slot);
        }
    }

    void ShopState::applyUpgrade(const ShopItem& item)
    {
        auto& profile = game->profile;

        if (item.targetStat == "fertilizer_regular") {
            profile.regularFertilizerCount++;
            return; 
        }
        else if (item.targetStat == "fertilizer_medium") {
            profile.mediumFertilizerCount++;
            return;
        }
        else if (item.targetStat == "fertilizer_best") {
            profile.bestFertilizerCount++;
            return;
        }

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

                if (backBtnSprite && backBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().popState();
                    return; 
                }

                for (auto& slot : uiSlots) {
                    if (!slot.soldOut && slot.bg.getGlobalBounds().contains(worldPos)) {
                        if (game->profile.spendCoins(slot.data.cost)) {
                            game->playUIClick();
                            applyUpgrade(slot.data);

                            slot.soldOut = true;
                            slot.bg.setFillColor(sf::Color(30, 30, 30, 200));
                            slot.bg.setOutlineColor(sf::Color(50, 50, 50));

                            if (slot.name) {
                                slot.name->setString(LocUTF8("ui_sold"));
                                slot.name->setOrigin({ slot.name->getLocalBounds().size.x / 2.0f, slot.name->getLocalBounds().size.y / 2.0f });
                            }
                            if (slot.name) slot.name->setFillColor(sf::Color(100, 100, 100));
                            if (slot.desc) slot.desc->setString("");
                            if (slot.cost) slot.cost->setString("");

                            if (slot.iconSprite) slot.iconSprite->setColor(sf::Color(100, 100, 100, 150));
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

    void ShopState::update(float /*dt*/)
    {
        // ZMIANA: Pobieramy COINS, a nie biomassJuice
        std::string coinStr = /*"Coins: " +*/ std::to_string(game->profile.coins);
        coinText->setString(coinStr);

        sf::FloatRect bBounds = coinText->getLocalBounds();
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        coinText->setOrigin({ std::round(bBounds.size.x / 2.0f), std::round(bBounds.position.y + bBounds.size.y / 2.0f) });
        coinText->setPosition({ viewSize.x / 2.0f, 110.f });

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        updateHover(backBtnSprite, { 60.0f, 60.0f }, mousePos);

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
        if (coinText) window.draw(*coinText);
        if (coinSprite.has_value()) window.draw(*coinSprite);

        for (const auto& slot : uiSlots) {
            window.draw(slot.bg);

            if (slot.iconSprite) window.draw(*slot.iconSprite);
            if (slot.name) window.draw(*slot.name);
            if (slot.desc) window.draw(*slot.desc);
            if (slot.cost) window.draw(*slot.cost);
        }
        if (backBtnSprite) window.draw(*backBtnSprite);

        game->drawMenuCursor();
    }
}