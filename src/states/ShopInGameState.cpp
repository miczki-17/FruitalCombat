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
#include <iostream>

namespace game::states
{
    using namespace game::core;

    ShopInGameState::ShopInGameState(game::Game* game)
        : State(game)
    {
        initUI();
        loadPool();
        rollItems(); // Wywo³ujemy po raz pierwszy, ale karty nie maj¹ jeszcze flag "locked"
    }

    void ShopInGameState::initUI()
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();

        // 1. Ekstremalnie ciemne, matowe t³o ca³ego sklepu (Minimalizm)
        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(15, 15, 18, 248));

        // 2. Tytu³ przeniesiony do lewego górnego rogu
        titleText.emplace(game->mainFont, "Shop", static_cast<int>(36 * GLOBAL_FONT_SCALE));
        titleText->setFillColor(sf::Color(240, 240, 240));
        titleText->setPosition({ 40.f, 30.f });

        // 3. Waluta na œrodku góry (Center Top)
        biomassText.emplace(game->mainFont, "", static_cast<int>(30 * GLOBAL_FONT_SCALE));
        biomassText->setFillColor(sf::Color(10, 230, 255));

        auto& rm = ResourceManager::get();
        if (rm.hasTexture("juice")) {
            biomassIcon.emplace(*rm.getTexture("juice"));
            biomassIcon->setScale({ 1.4f, 1.4f });
            biomassIcon->setOrigin({ biomassIcon->getTexture().getSize().x / 2.f, biomassIcon->getTexture().getSize().y / 2.f });
        }

        if (!rm.hasTexture("ui_empty_button")) {
            rm.loadTexture("ui_empty_button", "assets/textures/ui/empty_button.png", AssetGroup::Global);
        }

        // 4. Przycisk REROLL w prawym górnym rogu (Styl Brotato)
        setupButton("ui_empty_button", rerollBtnSprite, { viewSize.x - 160.f, 60.f }, { 220.f, 55.f });
        setupButtonText(rerollText, "REROLL - " + std::to_string(rerollCost), { viewSize.x - 160.f, 55.f }, 20);

        // 5. Przycisk GO (Resume) w prawym dolnym rogu
        setupButton("ui_empty_button", resumeBtnSprite, { viewSize.x - 160.f, viewSize.y - 60.f }, { 220.f, 60.f });
        std::string nextWaveStr = "GO (WAVE " + std::to_string(game->currentWaveNum) + ")";
        setupButtonText(resumeText, nextWaveStr, { viewSize.x - 160.f, viewSize.y - 65.f }, 22);
    }

    void ShopInGameState::loadPool()
    {
        commonPool.clear();
        epicPool.clear();
        uniquePool.clear();

        // --- ZAAWANSOWANE £ADOWANIE Z TABLICY EFEKTÓW ---
        if (game->upgradesConfig.contains("Common")) {
            for (const auto& up : game->upgradesConfig["Common"]) {
                ShopInGameItem newItem;
                newItem.id = up.value("id", "unknown");
                newItem.name = up.value("name", "Unknown");
                newItem.desc = up.value("desc", "");
                newItem.cost = up.value("cost", 20);
                newItem.iconKey = up.value("iconKey", "");
                newItem.rarity = ItemRarity::Common;

                if (up.contains("effects")) {
                    for (const auto& eff : up["effects"]) {
                        newItem.effects.push_back({ eff.value("stat", ""), eff.value("value", 0.0f) });
                    }
                }
                commonPool.push_back(newItem);
            }
        }
        if (game->upgradesConfig.contains("Epic")) {
            for (const auto& up : game->upgradesConfig["Epic"]) {
                ShopInGameItem newItem;
                newItem.id = up.value("id", "unknown");
                newItem.name = up.value("name", "Unknown");
                newItem.desc = up.value("desc", "");
                newItem.cost = up.value("cost", 80);
                newItem.iconKey = up.value("iconKey", "");
                newItem.rarity = ItemRarity::Epic;

                if (up.contains("effects")) {
                    for (const auto& eff : up["effects"]) {
                        newItem.effects.push_back({ eff.value("stat", ""), eff.value("value", 0.0f) });
                    }
                }
                epicPool.push_back(newItem);
            }
        }

        // Analogicznie dla Mythic
        if (game->upgradesConfig.contains("Mythic")) {
            for (const auto& up : game->upgradesConfig["Mythic"]) {
                ShopInGameItem newItem;
                newItem.id = up.value("id", "unknown");
                newItem.name = up.value("name", "Unknown");
                newItem.desc = up.value("desc", "");
                newItem.cost = up.value("cost", 150);
                newItem.iconKey = up.value("iconKey", "");
                newItem.rarity = ItemRarity::Mythic;

                if (up.contains("effects")) {
                    for (const auto& eff : up["effects"]) {
                        newItem.effects.push_back({ eff.value("stat", ""), eff.value("value", 0.0f) });
                    }
                }
                // Uwaga: Zrzucamy na razie do uniquePool, poniewa¿ algorytm losuje je w pierwszej kolejnoœci
                uniquePool.push_back(newItem);
            }
        }
    }

    void ShopInGameState::rollItems()
    {
        // 1. Zapisujemy zablokowane karty (Mechanika K£ÓDEK)
        std::vector<UIInGameItem> lockedCards;
        for (const auto& slot : uiSlots) {
            if (slot.isLocked && !slot.soldOut) {
                lockedCards.push_back(slot);
            }
        }

        uiSlots.clear();
        currentDisplay.clear();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> chanceDist(1, 100);

        auto tempCommon = commonPool;
        auto tempEpic = epicPool;
        auto tempUnique = uniquePool;

        // --- 4 KARTY ---
        int maxCards = 4;
        int itemsToRoll = maxCards - lockedCards.size();

        // 2. Odtwarzamy zablokowane 
        for (const auto& locked : lockedCards) {
            currentDisplay.push_back(locked.data);
        }

        // 3. Dobieramy puste miejsca
        for (int i = 0; i < itemsToRoll; ++i) {
            int roll = chanceDist(gen); // Losuje liczbê od 1 do 100
            ShopInGameItem selectedItem;

            // --- KONFIGURACJA SZANS DROPÓW ---
            // Od 1 do 5 (Szansa: 5%)
            if (roll <= 1 && !tempUnique.empty()) {
                std::uniform_int_distribution<size_t> dist(0, tempUnique.size() - 1);
                size_t idx = dist(gen);
                selectedItem = tempUnique[idx];
                tempUnique.erase(tempUnique.begin() + idx);
            }
            // Od 6 do 25 (Szansa: 20%)
            else if (roll <= 25 && !tempEpic.empty()) {
                std::uniform_int_distribution<size_t> dist(0, tempEpic.size() - 1);
                size_t idx = dist(gen);
                selectedItem = tempEpic[idx];
                tempEpic.erase(tempEpic.begin() + idx);
            }
            // Od 26 do 100 (Szansa: 75%)
            else if (!tempCommon.empty()) {
                std::uniform_int_distribution<size_t> dist(0, tempCommon.size() - 1);
                size_t idx = dist(gen);
                selectedItem = tempCommon[idx];
                tempCommon.erase(tempCommon.begin() + idx);
            }
            // Zabezpieczenie: jeœli skoñczy³y siê zwyk³e, a s¹ jeszcze epickie, daj epick¹
            else if (!tempEpic.empty()) {
                selectedItem = tempEpic[0];
                tempEpic.erase(tempEpic.begin());
            }

            currentDisplay.push_back(selectedItem);
        }

        // 4. MINIMALISTYCZNY RENDER KART (4 KARTY, SLIM DESIGN)
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();

        // Zmienione proporcje: wê¿sze i wy¿sze (bardziej prostok¹tne)
        float cardWidth = 220.f;
        float cardHeight = 350.f;
        float spacingX = 15.f;

        float leftZoneWidth = viewSize.x - 290.f;

        float totalCardsWidth = (4 * cardWidth) + (3 * spacingX);
        float startX = (leftZoneWidth / 2.0f) - (totalCardsWidth / 2.0f) + (cardWidth / 2.0f);
        float centerY = viewSize.y / 2.0f;

        auto& rm = ResourceManager::get();

        for (int i = 0; i < currentDisplay.size(); ++i) {
            UIInGameItem ui;
            ui.data = currentDisplay[i];

            for (const auto& locked : lockedCards) {
                if (locked.data.id == ui.data.id) {
                    ui.isLocked = true;
                    break;
                }
            }

            ui.bg.setSize({ cardWidth, cardHeight });
            ui.bg.setOrigin({ cardWidth / 2.0f, cardHeight / 2.0f });
            ui.bg.setPosition({ startX + (i * (cardWidth + spacingX)), centerY });

            ui.bg.setFillColor(sf::Color(22, 22, 25, 255));
            ui.bg.setOutlineThickness(2.f);

            sf::Color rarityColor = sf::Color(100, 150, 255);
            if (ui.data.rarity == ItemRarity::Epic) rarityColor = sf::Color(180, 50, 255);
            else if (ui.data.rarity == ItemRarity::Mythic) rarityColor = sf::Color(255, 50, 100);
            else if (ui.data.rarity == ItemRarity::Unique) rarityColor = sf::Color(255, 215, 0);

            ui.bg.setOutlineColor(rarityColor);

            sf::Vector2f innerTopLeft = ui.bg.getPosition() - sf::Vector2f(cardWidth / 2.0f, cardHeight / 2.0f);

            // czcionka
            setupButtonText(ui.name, ui.data.name, { innerTopLeft.x + 15.f, innerTopLeft.y + 25.f }, 18);
            ui.name->setFillColor(rarityColor);
            ui.name->setOrigin({ 0.f, ui.name->getOrigin().y }); // Left-Align

            // Opis
            setupButtonText(ui.desc, ui.data.desc, { innerTopLeft.x + 15.f, innerTopLeft.y + 80.f }, 15);
            ui.desc->setFillColor(sf::Color(200, 200, 200));
            ui.desc->setOrigin({ 0.f, 0.f }); // Left-Align top

            // Koszt (Zmniejszony margines dopasowany do wê¿szej karty)
            setupButtonText(ui.cost, std::to_string(ui.data.cost), { ui.bg.getPosition().x - 10.f, innerTopLeft.y + cardHeight - 35.f }, 20);
            ui.cost->setFillColor(sf::Color(10, 230, 255));

            if (rm.hasTexture("juice")) {
                ui.costIcon.emplace(*rm.getTexture("juice"));
                ui.costIcon->setScale({ 1.1f, 1.1f });
                ui.costIcon->setOrigin({ ui.costIcon->getTexture().getSize().x / 2.f, ui.costIcon->getTexture().getSize().y / 2.f });
                ui.costIcon->setPosition({ ui.bg.getPosition().x + 25.f, innerTopLeft.y + cardHeight - 35.f });
            }

            // K³ódka (Lock button) - wyœrodkowana na nowej szerokoœci
            ui.lockBtn.setSize({ 80.f, 24.f });
            ui.lockBtn.setOrigin({ 40.f, 0.f });
            ui.lockBtn.setPosition({ ui.bg.getPosition().x, innerTopLeft.y + cardHeight + 5.f });
            ui.lockBtn.setFillColor(ui.isLocked ? sf::Color(255, 215, 0, 200) : sf::Color(40, 40, 45, 255));

            setupButtonText(ui.lockText, ui.isLocked ? "LOCKED" : "Lock", { ui.lockBtn.getPosition().x, ui.lockBtn.getPosition().y + 10.f }, 12);
            ui.lockText->setFillColor(ui.isLocked ? sf::Color::Black : sf::Color(150, 150, 150));

            uiSlots.push_back(std::move(ui));
        }
    }

    void ShopInGameState::applyUpgrade(const ShopInGameItem& item)
    {
        auto* playerStats = game->arenaContext.playerStats;
        if (!playerStats) return;

        // --- SILNIK APLIKUJ¥CY TABLICÊ EFEKTÓW ---
        for (const auto& effect : item.effects)
        {
            if (effect.stat == "hp") {
                playerStats->increaseMaxHealth(effect.value);
            }
            else if (effect.stat == "mana") {
                playerStats->increaseMaxMana(effect.value);
            }
            else if (effect.stat == "mana_regen") {
                playerStats->increaseManaRegen(effect.value);
            }
            else if (effect.stat == "speed_pct") {
                playerStats->multiplyBaseSpeed(1.0f + effect.value);
            }
            else if (effect.stat == "atk_speed_pct") {
                playerStats->multiplyAttackSpeed(1.0f + effect.value);
            }
            else if (effect.stat == "dmg_pct") {
                playerStats->addDamageMultiplier(effect.value);
            }
            else if (effect.stat == "lifesteal") {
                playerStats->addLifesteal(effect.value);
            }
            else if (effect.stat == "armor_pct") {
                playerStats->addArmor(effect.value);
            }
            else if (effect.stat == "ult_rate") {
                playerStats->increaseUltChargeRate(effect.value);
            }
            else if (effect.stat == "bonusProjectiles") {
                playerStats->setBonusProjectiles(playerStats->getBonusProjectiles() + static_cast<int>(effect.value));
            }
        }
    }

    void ShopInGameState::handleEvent(const sf::Event& event)
    {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(mousePos, game->getWindow().getDefaultView());

                // Akcja K³ódki (Zamro¿enia)
                for (auto& slot : uiSlots) {
                    if (!slot.soldOut && slot.lockBtn.getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        slot.isLocked = !slot.isLocked; // Prze³¹cz stan
                        slot.lockBtn.setFillColor(slot.isLocked ? sf::Color::Yellow : sf::Color(100, 100, 100));
                        slot.lockText->setString(slot.isLocked ? "LOCKED" : "LOCK");
                        return; // Opuœæ po znalezieniu
                    }
                }

                if (rerollBtnSprite && rerollBtnSprite->getGlobalBounds().contains(worldPos)) {
                    if (game->profile.biomassJuice >= rerollCost) {
                        game->playUIClick();
                        game->profile.spendJuice(rerollCost);
                        rollItems();

                        // INFLACJA SKLEPU
                        rerollCost += 10;
                        rerollText->setString("Reroll (" + std::to_string(rerollCost) + ")");
                    }
                }

                if (resumeBtnSprite && resumeBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().popState();
                }

                // Zakup KARTY
                for (auto& slot : uiSlots) {
                    if (!slot.soldOut && slot.bg.getGlobalBounds().contains(worldPos)) {
                        if (game->profile.biomassJuice >= slot.data.cost) {
                            game->playUIClick();
                            game->profile.spendJuice(slot.data.cost);
                            applyUpgrade(slot.data);

                            slot.soldOut = true;
                            slot.isLocked = false; // Po kupnie zdejmujemy k³ódkê na wszelki wypadek
                            slot.lockBtn.setFillColor(sf::Color::Transparent); // Ukryj
                            slot.lockText->setString("");

                            slot.bg.setFillColor(sf::Color(30, 30, 30, 200));
                            slot.bg.setOutlineColor(sf::Color(50, 50, 50));
                            slot.name->setString("SOLD");
                            slot.name->setFillColor(sf::Color(100, 100, 100));
                            slot.desc->setString("");
                            slot.cost->setString("");

                            if (slot.costIcon) {
                                slot.costIcon->setColor(sf::Color(255, 255, 255, 0));
                            }
                        }
                    }
                }
            }
        }
    }

    void ShopInGameState::update(float /*dt*/)
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();

        biomassText->setString(std::to_string(game->profile.biomassJuice));
        sf::FloatRect bBounds = biomassText->getLocalBounds();
        biomassText->setOrigin({ 0.f, std::round(bBounds.position.y + bBounds.size.y / 2.0f) });

        float totalWidth = 35.f + bBounds.size.x;
        float startX = (viewSize.x / 2.f) - (totalWidth / 2.f); // Prawdziwy œrodek ekranu

        // Ikona i waluta na œrodku na samej górze
        if (biomassIcon) biomassIcon->setPosition({ startX + 15.f, 55.f });
        biomassText->setPosition({ startX + 45.f, 55.f });

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        updateHover(rerollBtnSprite, { 250.0f, 60.0f }, mousePos, &rerollText);
        updateHover(resumeBtnSprite, { 250.0f, 60.0f }, mousePos, &resumeText);

        for (auto& slot : uiSlots) {
            if (!slot.soldOut) {
                if (slot.bg.getGlobalBounds().contains(mousePos)) {
                    // Hover rozjaœnia obwódkê
                    slot.bg.setOutlineColor(sf::Color(255, 255, 255, 200));
                }
                else {
                    if (slot.data.rarity == ItemRarity::Common) slot.bg.setOutlineColor(sf::Color(40, 150, 255));
                    else if (slot.data.rarity == ItemRarity::Epic) slot.bg.setOutlineColor(sf::Color(180, 50, 255));
                    else if (slot.data.rarity == ItemRarity::Mythic) slot.bg.setOutlineColor(sf::Color(255, 50, 100));
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

            if (!slot.soldOut) {
                window.draw(slot.lockBtn);
                if (slot.lockText) window.draw(*slot.lockText);
            }
        }

        // --- MINIMALISTYCZNY PANEL STATYSTYK (BROTATO STYLE) ---
        auto* stats = game->arenaContext.playerStats;
        if (stats)
        {
            sf::Vector2f viewSize = window.getDefaultView().getSize();

            // Wê¿szy, wy¿szy panel
            float sidebarWidth = 240.f;
            float sidebarHeight = 440.f;
            float sidebarX = viewSize.x - sidebarWidth - 20.f;
            float sidebarY = viewSize.y / 2.0f - (sidebarHeight / 2.0f);

            // Ca³kowicie p³askie, ciemne t³o (bez grubych ramek)
            sf::RectangleShape statsBg({ sidebarWidth, sidebarHeight });
            statsBg.setPosition({ sidebarX, sidebarY });
            statsBg.setFillColor(sf::Color(22, 22, 25, 255));
            window.draw(statsBg);

            // Wyœrodkowany, ma³y tytu³ "stats"
            sf::Text panelTitle(game->mainFont, "stats", static_cast<int>(20 * GLOBAL_FONT_SCALE));
            panelTitle.setFillColor(sf::Color(240, 240, 240));
            sf::FloatRect titleBounds = panelTitle.getLocalBounds();
            panelTitle.setPosition({ sidebarX + (sidebarWidth / 2.0f) - (titleBounds.size.x / 2.0f), sidebarY + 15.f });
            window.draw(panelTitle);

            // Struktura przechowuj¹ca: Nazwê, Wartoœæ jako String, Kolor
            struct StatRow {
                std::string name;
                std::string valStr;
                sf::Color color;
            };
            std::vector<StatRow> statsLines;

            // --- FUNKCJA POMOCNICZA DO FORMATOWANIA (Brotato Style) ---
            // Jeœli 0 -> "0", jeœli > 0 -> np. "+15%", jeœli < 0 -> "-5%"
            auto formatStat = [](int val, bool isPct) {
                if (val == 0) return std::string("0");
                std::string prefix = val > 0 ? "+" : "";
                std::string suffix = isPct ? "%" : "";
                return prefix + std::to_string(val) + suffix;
            };

            // Paleta kolorów
            sf::Color colNeutral = sf::Color(180, 180, 180); // Szary dla 0
            sf::Color colBuff = sf::Color(80, 255, 120);     // Zieleñ
            sf::Color colNerf = sf::Color(255, 80, 80);      // Czerwieñ
            sf::Color colWhite = sf::Color::White;

            // Obliczenia wartoœci
            int hpVal = static_cast<int>(stats->getMaxHealth());
            int manaVal = static_cast<int>(stats->getMaxMana());
            int dmgBonus = static_cast<int>(std::round((stats->getDamageMultiplier() - 1.0f) * 100.f));
            int speedBonus = static_cast<int>(std::round((stats->getCurrentSpeed() / 200.0f - 1.0f) * 100.f));
            int atkSpeedBonus = static_cast<int>(std::round((stats->getAttackSpeed() - 1.0f) * 100.f));
            int lifestealVal = static_cast<int>(stats->getLifesteal() * 100.f);
            int armorVal = static_cast<int>(stats->getArmor() * 100.f);
            int projVal = stats->getBonusProjectiles();

            // Wype³nianie listy (Nazwa, Wartoœæ, Kolor)
            statsLines.push_back({ "Max HP", std::to_string(hpVal), colWhite });
            statsLines.push_back({ "Max Mana", std::to_string(manaVal), colWhite });
            statsLines.push_back({ "% Damage", formatStat(dmgBonus, true), dmgBonus > 0 ? colBuff : (dmgBonus < 0 ? colNerf : colNeutral) });
            statsLines.push_back({ "% Speed", formatStat(speedBonus, true), speedBonus > 0 ? colBuff : (speedBonus < 0 ? colNerf : colNeutral) });
            statsLines.push_back({ "% Attack Speed", formatStat(atkSpeedBonus, true), atkSpeedBonus > 0 ? colBuff : (atkSpeedBonus < 0 ? colNerf : colNeutral) });
            statsLines.push_back({ "% Lifesteal", formatStat(lifestealVal, true), lifestealVal > 0 ? colBuff : colNeutral });
            statsLines.push_back({ "Armor", formatStat(armorVal, false), armorVal > 0 ? colBuff : colNeutral });
            statsLines.push_back({ "Projectiles", formatStat(projVal, false), projVal > 0 ? colBuff : colNeutral });

            // --- RYSOWANIE W DWÓCH KOLUMNACH ---
            float startY = sidebarY + 65.f;
            float rowHeight = 26.f; // Zmniejszony odstêp miêdzy wierszami
            float paddingX = 20.f;  // Margines boczny

            for (size_t i = 0; i < statsLines.size(); ++i)
            {
                int fontSize = static_cast<int>(15 * GLOBAL_FONT_SCALE); // Zmniejszona czcionka!

                // NAZWA (Wyrównana do lewej)
                sf::Text nameText(game->mainFont, statsLines[i].name, fontSize);
                nameText.setFillColor(statsLines[i].color);
                nameText.setPosition({ sidebarX + paddingX, startY + (i * rowHeight) });
                window.draw(nameText);

                // WARTOŒÆ (Wyrównana do prawej)
                sf::Text valText(game->mainFont, statsLines[i].valStr, fontSize);
                valText.setFillColor(statsLines[i].color);
                sf::FloatRect valBounds = valText.getLocalBounds();

                // Pozycja X = Prawa krawêdŸ panelu - margines - szerokoœæ tekstu
                float rightAlignX = sidebarX + sidebarWidth - paddingX - valBounds.size.x;
                valText.setPosition({ rightAlignX, startY + (i * rowHeight) });
                window.draw(valText);
            }
        }

        if (rerollBtnSprite) window.draw(*rerollBtnSprite);
        if (rerollText) window.draw(*rerollText);

        if (resumeBtnSprite) window.draw(*resumeBtnSprite);
        if (resumeText) window.draw(*resumeText);

        game->drawMenuCursor();
    }
}