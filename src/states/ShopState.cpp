// --- ShopState.cpp ---

#include "ShopState.h"
#include "../components/StatsComponent.h"
#include "../core/ArenaContext.h"
#include "../core/Game.h"
#include <random>
#include <algorithm>

namespace game::states
{
    ShopState::ShopState(game::Game* game)
        : State(game),
        titleText(game->mainFont),
        biomassText(game->mainFont),
        rerollText(game->mainFont)
    {
        titleText.setString("MUTATION STATION");
        titleText.setCharacterSize(40);
        titleText.setPosition({ 50.f, 30.f });

        biomassText.setCharacterSize(24);
        biomassText.setPosition({ 50.f, 80.f });

        rerollButton.setSize({ 200.f, 50.f });
        rerollButton.setPosition({ 50.f, 500.f });
        rerollButton.setFillColor(sf::Color(100, 100, 150));

        rerollText.setString("Reroll (30 Juice)");
        rerollText.setCharacterSize(20);
        rerollText.setPosition({ 60.f, 510.f });

        loadPool();
        rollItems();
    }

    void ShopState::loadPool()
    {
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
        for (int i = 0; i < numItems; ++i) {
            currentDisplay.push_back(tempPool[i]);

            UIItem ui(game->mainFont);
            ui.data = tempPool[i];
            ui.bg.setSize({ 300.f, 100.f });

            ui.bg.setPosition({ 50.f, 150.f + (static_cast<float>(i) * 110.f) });
            ui.bg.setFillColor(sf::Color(50, 50, 50));
            ui.bg.setOutlineThickness(2.f);

            ui.name.setString(ui.data.name);
            ui.name.setCharacterSize(22);
            ui.name.setPosition(ui.bg.getPosition() + sf::Vector2f(10.f, 10.f));

            ui.desc.setString(ui.data.desc);
            ui.desc.setCharacterSize(16);
            ui.desc.setPosition(ui.bg.getPosition() + sf::Vector2f(10.f, 40.f));

            ui.cost.setString(std::to_string(ui.data.cost) + " Juice");
            ui.cost.setCharacterSize(18);
            ui.cost.setFillColor(sf::Color::Yellow);
            ui.cost.setPosition(ui.bg.getPosition() + sf::Vector2f(10.f, 70.f));

            uiSlots.push_back(ui);
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

    StateType ShopState::getType() const { return StateType::Shop; }

    void ShopState::handleEvent(const sf::Event& event)
    {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(mousePos);

                // reroll
                if (rerollButton.getGlobalBounds().contains(worldPos)) {
                    if (game->profile.biomassJuice >= rerollCost) {
                        game->profile.addJuice(-rerollCost);
                        rollItems();
                    }
                }

				// buying items
                for (auto& slot : uiSlots) {
                    if (!slot.soldOut && slot.bg.getGlobalBounds().contains(worldPos)) {
                        if (game->profile.biomassJuice >= slot.data.cost) {
                            game->profile.spendJuice(slot.data.cost);
                            applyUpgrade(slot.data);
                            slot.soldOut = true;
                            slot.bg.setFillColor(sf::Color(30, 30, 30));
                            slot.name.setString("SOLD");
                            slot.desc.setString("");
                            slot.cost.setString("");
                        }
                    }
                }
            }
        }

        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			// close shop on Escape or Space
            if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Space) {
                game->getStateMachine().popState();
            }
        }
    }

    void ShopState::update(float dt) {
        biomassText.setString("Juice Available: " + std::to_string(game->profile.biomassJuice));
    }

    void ShopState::render(sf::RenderWindow& window) {
        window.draw(titleText);
        window.draw(biomassText);

        for (const auto& slot : uiSlots) {
            window.draw(slot.bg);
            window.draw(slot.name);
            window.draw(slot.desc);
            window.draw(slot.cost);
        }

        window.draw(rerollButton);
        window.draw(rerollText);
    }
}