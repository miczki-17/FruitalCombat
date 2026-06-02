// --- DeathState.cpp ---

#include "DeathState.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h"
#include "../core/AudioManager.h"
#include "../core/LocalizationManager.h"
#include <iostream>
#include <cmath>

namespace game::states
{
    using namespace game::core;

    DeathState::DeathState(game::Game* game) : State(game)
    {
        std::cout << "[DeathState] Switched to DeathScreen.\n";
        AudioManager::get().stopMusic();

        auto& rm = ResourceManager::get();
        if (!rm.hasTexture("ui_empty_button")) {
            rm.loadTexture("ui_empty_button", "assets/textures/ui/empty_button.png", AssetGroup::Global);
        }
        if (!rm.hasTexture("magic_bean_base")) {
            rm.loadTexture("magic_bean_base", "assets/textures/entities/drops/magic_bean_base.png", AssetGroup::Global);
        }

        initUI();
    }

    void DeathState::initUI()
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        float centerX = viewSize.x / 2.0f;

        // overlay
        backgroundOverlay_.setSize(viewSize);
        backgroundOverlay_.setFillColor(sf::Color(25, 0, 0, 200));

        initLabels(centerX, viewSize.y);
        initButtons(centerX, viewSize.y);
    }

    void DeathState::initLabels(float centerX, float viewHeight)
    {
        youDiedText_.emplace(game->mainFont, LocUTF8("ui_you_died"), static_cast<int>(65 * GLOBAL_FONT_SCALE));
        youDiedText_->setFillColor(sf::Color(220, 40, 40));
        youDiedText_->setStyle(sf::Text::Bold);
        sf::FloatRect ydBounds = youDiedText_->getLocalBounds();
        youDiedText_->setOrigin({ std::round(ydBounds.size.x / 2.0f), std::round(ydBounds.position.y + ydBounds.size.y / 2.0f) });
        youDiedText_->setPosition({ centerX, viewHeight * 0.18f });

        float currentY = viewHeight * 0.38f;

        // --- KILLER ---
        killedByLabel_.emplace(game->mainFont, LocUTF8("ui_killed_by"), static_cast<int>(18 * GLOBAL_FONT_SCALE));
        killedByLabel_->setFillColor(sf::Color(180, 180, 180));
        sf::FloatRect kbBounds = killedByLabel_->getLocalBounds();
        killedByLabel_->setOrigin({ std::round(kbBounds.size.x / 2.0f), std::round(kbBounds.position.y + kbBounds.size.y / 2.0f) });
        killedByLabel_->setPosition({ centerX, currentY });

        currentY += 35.0f;

        killerNameText_.emplace(game->mainFont, LocUTF8(game->lastSessionResults.killerNameKey), static_cast<int>(30 * GLOBAL_FONT_SCALE));
        killerNameText_->setFillColor(sf::Color::White);
        killerNameText_->setStyle(sf::Text::Italic);
        sf::FloatRect knBounds = killerNameText_->getLocalBounds();
        killerNameText_->setOrigin({ std::round(knBounds.size.x / 2.0f), std::round(knBounds.position.y + knBounds.size.y / 2.0f) });
        killerNameText_->setPosition({ centerX, currentY });

        currentY += 80.0f;

        // --- STATISTICS ---
        auto& rm = ResourceManager::get();
        if (rm.hasTexture("magic_bean_base")) {
            biomassIcon_.emplace(*rm.getTexture("magic_bean_base"));
            sf::Vector2u iconSize = biomassIcon_->getTexture().getSize();
            biomassIcon_->setOrigin({ iconSize.x / 2.0f, iconSize.y / 2.0f });
            biomassIcon_->setScale({ 2.5f, 2.5f });
            biomassIcon_->setPosition({ centerX - 30.0f, currentY });
        }

        // RESULTS
        scoreText_.emplace(game->mainFont, std::to_string(game->lastSessionResults.biomassCollected), static_cast<int>(24 * GLOBAL_FONT_SCALE));
        scoreText_->setFillColor(sf::Color::White);
        scoreText_->setStyle(sf::Text::Bold);
        sf::FloatRect scBounds = scoreText_->getLocalBounds();
        scoreText_->setOrigin({ 0.0f, std::round(scBounds.position.y + scBounds.size.y / 2.0f) });
        scoreText_->setPosition({ centerX + 5.0f, currentY - 5.0f });

        currentY += 45.0f;

        // WAVES NUM
        wavesText_.emplace(game->mainFont, LocUTF8("ui_waves") + " " + std::to_string(game->lastSessionResults.wavesSurvived), static_cast<int>(18 * GLOBAL_FONT_SCALE));
        wavesText_->setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect wBounds = wavesText_->getLocalBounds();
        wavesText_->setOrigin({ std::round(wBounds.size.x / 2.0f), std::round(wBounds.position.y + wBounds.size.y / 2.0f) });
        wavesText_->setPosition({ centerX, currentY });
    }

    void DeathState::initButtons(float centerX, float viewHeight)
    {
        float btnY = viewHeight * 0.72f;

        setupButton("ui_empty_button", retryBtnSprite_, { centerX, btnY }, { 200.0f, 60.0f });
        setupButtonText(retryBtnText_, LocUTF8("ui_retry"), { centerX, btnY - 5.0f }, 24);

        btnY += 80.0f;

        setupButton("ui_empty_button", menuBtnSprite_, { centerX, btnY }, { 200.0f, 60.0f });
        setupButtonText(menuBtnText_, LocUTF8("ui_menu"), { centerX, btnY - 5.0f }, 24);
    }

    void DeathState::handleEvent(const sf::Event& event)
    {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
                    sf::Mouse::getPosition(game->getWindow()),
                    game->getWindow().getDefaultView());

                if (retryBtnSprite_ && retryBtnSprite_->getGlobalBounds().contains(mousePos)) {
                    game->playUIClick();
                    game->getWindow().setMouseCursorVisible(false);

                    // Bezpieczne przeladowanie gry
                    game->getStateMachine().popState();
                    game->getStateMachine().changeState(StateType::Playing);
                    return;
                }

                if (menuBtnSprite_ && menuBtnSprite_->getGlobalBounds().contains(mousePos)) {
                    game->playUIClick();

                    game->getStateMachine().popState();
                    game->getStateMachine().changeState(StateType::Intro);
                    return;
                }
            }
        }
    }

    void DeathState::update(float /*dt*/)
    {
        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        updateHover(retryBtnSprite_, { 200.0f, 60.0f }, mousePos, &retryBtnText_);
        updateHover(menuBtnSprite_, { 200.0f, 60.0f }, mousePos, &menuBtnText_);
    }

    void DeathState::render(sf::RenderWindow& window)
    {
        window.draw(backgroundOverlay_);

        if (youDiedText_) window.draw(*youDiedText_);
        if (killedByLabel_) window.draw(*killedByLabel_);
        if (killerNameText_) window.draw(*killerNameText_);

        if (biomassIcon_) window.draw(*biomassIcon_);
        if (scoreText_) window.draw(*scoreText_);
        if (wavesText_) window.draw(*wavesText_);

        if (retryBtnSprite_) window.draw(*retryBtnSprite_);
        if (retryBtnText_) window.draw(*retryBtnText_);

        if (menuBtnSprite_) window.draw(*menuBtnSprite_);
        if (menuBtnText_) window.draw(*menuBtnText_);

        game->drawMenuCursor();
    }
}