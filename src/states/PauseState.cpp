// --- PauseState.cpp ---

#include "PauseState.h"
#include "../core/ResourceManager.h"
#include "../core/Game.h"
#include "../core/LocalizationManager.h"
#include <iostream>
#include <cmath> 

namespace game::states
{
    using namespace game::core;

    PauseState::PauseState(game::Game* game) : State(game)
    {
        initUI();
    }

    void PauseState::initUI()
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        // overlay
        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(0, 0, 0, 180));

        // --- EXIT TEXT ---
        confirmText.emplace(game->mainFont, LocUTF8("ui_exit_confirm"), static_cast<int>(36 * GLOBAL_FONT_SCALE));
        confirmText->setFillColor(sf::Color::White);
        sf::FloatRect textBounds = confirmText->getLocalBounds();
        confirmText->setOrigin({
            std::round(textBounds.size.x / 2.0f),
            std::round(textBounds.position.y + textBounds.size.y / 2.0f)
            });
        confirmText->setPosition({ centerX, centerY - 100.0f });

        // --- G£OWNE PRZYCISKI MENU PAUZY ---
        setupButton("ui_empty_button", resumeBtn, { centerX, centerY - 80.0f }, { 200.0f, 60.0f });
        setupButton("ui_empty_button", settingsBtn, { centerX, centerY }, { 200.0f, 60.0f });
        setupButton("ui_empty_button", exitBtn, { centerX, centerY + 80.0f }, { 200.0f, 60.0f });

        setupButtonText(resumeText, LocUTF8("ui_resume"), { centerX, centerY - 80.0f });
        setupButtonText(settingsText, LocUTF8("ui_settings"), { centerX, centerY });
        setupButtonText(exitText, LocUTF8("ui_exit"), { centerX, centerY + 80.0f });

        // --- PRZYCISKI POTWIERDZENIA WYJSCIA ---
        setupButton("ui_empty_button", yesBtn, { centerX - 120.0f, centerY }, { 150.0f, 60.0f });
        setupButton("ui_empty_button", noBtn, { centerX + 120.0f, centerY }, { 150.0f, 60.0f });

        setupButtonText(yesText, LocUTF8("ui_yes"), { centerX - 120.0f, centerY });
        setupButtonText(noText, LocUTF8("ui_no"), { centerX + 120.0f, centerY });

        lastLangCode = game::core::LocalizationManager::get().getCurrentLanguageCode();
    }

    void PauseState::handleEvent(const sf::Event& event)
    {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                if (showExitConfirm) {
                    showExitConfirm = false; // Reset pytania, wracamy do g³ownego menu pauzy
                }
                else {
                    game->getStateMachine().popState(); // Zdjecie nakladki return into game
                }
                return;
            }
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(
                    sf::Mouse::getPosition(game->getWindow()),
                    game->getWindow().getDefaultView());

                if (showExitConfirm)
                {
                    if (yesBtn && yesBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        game->getStateMachine().popState();
                        game->getStateMachine().changeState(StateType::Intro);
                        return;
                    }
                    if (noBtn && noBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        showExitConfirm = false; // Wroc do standardowej pauzy
                        return;
                    }
                }
                else
                {
                    if (resumeBtn && resumeBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        game->getStateMachine().popState(); // Zdejmujemy pauze
                        return;
                    }
                    if (settingsBtn && settingsBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        game->getStateMachine().pushState(StateType::Settings);
                        return;
                    }
                    if (exitBtn && exitBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        showExitConfirm = true; // Prze³acz na widok zapytania
                        return;
                    }
                }
            }
        }
    }

    void PauseState::update(float /*dt*/)
    {
        std::string currentLang = game::core::LocalizationManager::get().getCurrentLanguageCode();
        if (lastLangCode != currentLang) {
            lastLangCode = currentLang;
            refreshTexts();
        }

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        if (showExitConfirm) {
            updateHover(yesBtn, { 150.0f, 60.0f }, mousePos, &yesText);
            updateHover(noBtn, { 150.0f, 60.0f }, mousePos, &noText);
        }
        else {
            updateHover(resumeBtn, { 200.0f, 60.0f }, mousePos, &resumeText);
            updateHover(settingsBtn, { 200.0f, 60.0f }, mousePos, &settingsText);
            updateHover(exitBtn, { 200.0f, 60.0f }, mousePos, &exitText);
        }
    }

    void PauseState::render(sf::RenderWindow& window)
    {
        // Upewniamy sie, ze renderujemy UI bez wplywu kamery poruszajacego siê gracza
        window.setView(window.getDefaultView());

        window.draw(darkOverlay);

        if (showExitConfirm) {
            if (confirmText) window.draw(*confirmText);

            if (yesBtn) {
                window.draw(*yesBtn);
                if (yesText) window.draw(*yesText);
            }
            if (noBtn) {
                window.draw(*noBtn);
                if (noText) window.draw(*noText);
            }
        }
        else {
            if (resumeBtn) {
                window.draw(*resumeBtn);
                if (resumeText) window.draw(*resumeText);
            }
            if (settingsBtn) {
                window.draw(*settingsBtn);
                if (settingsText) window.draw(*settingsText);
            }
            if (exitBtn) {
                window.draw(*exitBtn);
                if (exitText) window.draw(*exitText);
            }
        }
        game->drawMenuCursor();
    }


    /////////////////////////////////

    void PauseState::refreshTexts()
    {
        // Lambda pomocnicza do b³yskawicznego aktualizowania i centrowania tekstu
        auto updateText = [](std::optional<sf::Text>& optText, const sf::String& str) {
            if (optText) {
                optText->setString(str);
                sf::FloatRect bounds = optText->getLocalBounds();
                optText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
            }
        };

        updateText(confirmText, LocUTF8("ui_exit_confirm"));
        updateText(resumeText, LocUTF8("ui_resume"));
        updateText(settingsText, LocUTF8("ui_settings"));
        updateText(exitText, LocUTF8("ui_exit"));
        updateText(yesText, LocUTF8("ui_yes"));
        updateText(noText, LocUTF8("ui_no"));
    }
}