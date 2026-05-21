#include "ShopState.h"
#include "../core/Game.h"

#include <iostream>
#include <stdexcept>

namespace game::states
{
    ShopState::ShopState(game::Game* game)
        : State(game)
    {

        titleText.emplace(game->mainFont);
        titleText->setString("MUTATION DECK - EVOLVE YOUR FRUIT");
        titleText->setCharacterSize(36);
        titleText->setPosition({ 200.f, 50.f });

        currencyText.emplace(game->mainFont);
        currencyText->setString("Juice Available: " + std::to_string(game->playerJuice));
        currencyText->setCharacterSize(24);
        currencyText->setPosition({ 200.f, 110.f });

        instructionText.emplace(game->mainFont);
        instructionText->setString("Press ESC to return to the Arena");
        instructionText->setCharacterSize(18);
        instructionText->setPosition({ 200.f, 700.f });

        std::cout << "[SHOP STATE] Opened upgrade deck.\n";

        setupUpgrades();
    }

    void ShopState::setupUpgrades()
    {
        std::vector<std::pair<std::string, int>> upgradeData =
        {
            { "Cardio (+5% Movement Speed)", 1 },
            { "Vitamins (+15 Max HP & Heal)", 2 },
            { "Frenzy (+6% Attack Speed)", 3 }
        };

        switch (game->selectedFruitType)
        {
        case game::entities::FruitType::Apple:
            upgradeData.push_back({
                "Adrenaline (Dash cooldown reduced by 0.15s)",
                4
                });
            break;

        case game::entities::FruitType::Banana:
            upgradeData.push_back({
                "Thick Peel (+8% RindRoll duration)",
                4
                });
            break;

        case game::entities::FruitType::Orange:
            upgradeData.push_back({
                "Burst Spread (+1 Pellet to Shotgun)",
                4
                });
            break;

        default:
            upgradeData.push_back({
                "Sharp Pips (+5% Bullet Size)",
                4
                });
            break;
        }

        float startY = 200.f;

        for (size_t i = 0; i < upgradeData.size(); ++i)
        {
            UpgradeOption opt;

            opt.typeId = upgradeData[i].second;
            opt.cost = 40 + static_cast<int>(i) * 10;

            opt.button.setSize({ 500.f, 60.f });

            opt.button.setPosition({
                200.f,
                startY + (static_cast<float>(i) * 90.f)
                });

            opt.button.setFillColor(
                sf::Color(40, 40, 50)
            );

            opt.button.setOutlineThickness(2.f);
            opt.button.setOutlineColor(sf::Color::White);

            // Tworzenie tekstu (SFML 3 / uniwersalne podej?cie)
            opt.text.emplace(game->mainFont);
            opt.text->setString(upgradeData[i].first + " | Cost: " + std::to_string(opt.cost) + " Juice");
            opt.text->setCharacterSize(20);
            opt.text->setPosition({
                220.f,
                startY + (static_cast<float>(i) * 90.f) + 15.f
                });

            options.push_back(std::move(opt));
        }
    }

    void ShopState::applyUpgrade(int typeId)
    {
        switch (typeId)
        {
        case 1:
            game->upgrade_speedMod += 0.05f;
            std::cout << "Speed upgraded!\n";
            break;

        case 2:
            game->upgrade_maxHpBonus += 15;
            std::cout << "HP upgraded!\n";
            break;

        case 3:
            game->upgrade_atkSpeedMod += 0.06f;
            std::cout << "Attack speed upgraded!\n";
            break;

        case 4:
            game->upgrade_uniqueCount++;
            std::cout << "Unique skill upgraded!\n";
            break;
        }
    }

    void ShopState::handleEvent(const sf::Event& event)
    {
        // ESC
        if (auto* keyPressed =
            event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code ==
                sf::Keyboard::Key::Escape)
            {
                game->getStateMachine().popState();
            }
        }

        // Mouse
        if (auto* mousePressed =
            event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button ==
                sf::Mouse::Button::Left)
            {
                sf::Vector2i mousePos =
                    sf::Mouse::getPosition(
                        game->getWindow()
                    );

                for (auto& opt : options)
                {
                    if (opt.button
                        .getGlobalBounds()
                        .contains(
                            static_cast<sf::Vector2f>(mousePos)
                        ))
                    {
                        if (game->playerJuice >= opt.cost)
                        {
                            game->playerJuice -= opt.cost;

                            applyUpgrade(opt.typeId);

                            currencyText->setString(
                                "Juice Available: " +
                                std::to_string(
                                    game->playerJuice
                                )
                            );

                            opt.button.setFillColor(
                                sf::Color(20, 80, 20)
                            );

                            break;
                        }
                    }
                }
            }
        }
    }

    void ShopState::update(float dt)
    {
    }

    void ShopState::render(sf::RenderWindow& window)
    {
        window.clear(sf::Color(15, 15, 25));

        window.draw(*titleText);
        window.draw(*currencyText);
        window.draw(*instructionText);

        for (auto& opt : options)
        {
            window.draw(opt.button);

            if (opt.text.has_value())
            {
                window.draw(*opt.text);
            }
        }
    }

    game::states::StateType ShopState::getType() const
    {
        return StateType::Shop;
    }
}