// --- MenuState.cpp ---

#include "MenuState.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h"
#include "../core/AudioManager.h"
#include "../core/LocalizationManager.h"
#include <format>
#include <iostream>
#include <cmath> 

namespace game::states
{
    using namespace game::core;

    MenuState::MenuState(game::Game* game)
        : State(game), currentFrame(1), totalFrames(6), elapsedTime(0.f)
    {
        initBackground();
        initButtons();

        // Ustawienia Audio
        sf::Listener::setGlobalVolume(35.0f);
        auto& audio = AudioManager::get();
        if (!audio.isMusicPlaying("bg_music")) {
            audio.playMusic("bg_music");
        }

        // language
        lastLangCode = LocalizationManager::get().getCurrentLanguageCode();
    }

    void MenuState::initBackground()
    {
        auto& rm = ResourceManager::get();
        sf::Vector2f viewSize = game->getWindow().getView().getSize();

        frameDuration = 1.0f / 10.0f; // 10 FPS dla t³a

        // Pobieranie wszystkich dostepnych klatek do wektora
        for (int i = 1; ; ++i) {
            std::string key = "bg_" + std::to_string(i);
            if (rm.hasTexture(key)) {
                bgTextures.push_back(rm.getTextureShared(key));
            }
            else {
                break;
            }
        }

        if (!bgTextures.empty()) {
            frameSprite.emplace(*bgTextures[0]);
            sf::Vector2f introSize(bgTextures[0]->getSize());
            frameSprite->setScale({ viewSize.x / introSize.x, viewSize.y / introSize.y });
            frameSprite->setPosition({ 0.f, 0.f });
        }
    }

    void MenuState::initButtons()
    {
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float margin = 20.0f;

        // setupButton , setupButtonText -> State.h
        setupButton("ui_empty_button", startBtnSprite, { centerX, 600.0f }, { 300.0f, 120.0f });
        setupButtonText(startText, LocUTF8("ui_start"), { centerX + 5.0f, 595.0f }, 60);
        startText->setOutlineColor(sf::Color::Black);
        startText->setOutlineThickness(5.8f);

        setupButton("ui_settings", settingsBtnSprite, { 0.0f, 0.0f }, { 60.0f, 60.0f });
        if (settingsBtnSprite) {
            sf::FloatRect bounds = settingsBtnSprite->getGlobalBounds();
            settingsBtnSprite->setPosition({ viewSize.x - margin - (bounds.size.x / 2.0f), margin + (bounds.size.y / 2.0f) });
        }
    }

    void MenuState::handleEvent(const sf::Event& event)
    {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(
                    sf::Mouse::getPosition(game->getWindow()),
                    game->getWindow().getDefaultView());

                if (startBtnSprite && startBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    // now game is running!
                    game->isGameRun = true;
                    game->getStateMachine().changeState(StateType::Lobby);
                    return;
                }
                if (settingsBtnSprite && settingsBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().pushState(StateType::Settings);
                    return;
                }
            }
        }
    }

    void MenuState::updateBackgroundAnimation(float dt)
    {
        if (bgTextures.empty() || !frameSprite.has_value()) return;

        elapsedTime += dt;
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        static bool isReverse = false;

        while (elapsedTime >= frameDuration) {
            elapsedTime -= frameDuration;

            if (!isReverse) currentFrame++; else currentFrame--;

            if (bgTextures.size() > 1) {
                if (currentFrame >= bgTextures.size()) { currentFrame = bgTextures.size(); isReverse = true; }
                else if (currentFrame <= 1) { currentFrame = 1; isReverse = false; }
            }
            else {
                currentFrame = 1;
            }

            const sf::Texture& nextTex = *bgTextures[currentFrame - 1];
            frameSprite->setTexture(nextTex, true);
            sf::Vector2f introSize(nextTex.getSize());
            frameSprite->setScale({ viewSize.x / introSize.x, viewSize.y / introSize.y });
        }
    }

    void MenuState::update(float dt)
    {
        std::string currentLang = game::core::LocalizationManager::get().getCurrentLanguageCode();
        if (lastLangCode != currentLang) {
            lastLangCode = currentLang;
            refreshTexts();
        }

        updateBackgroundAnimation(dt);

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        // Hover -> State.cpp
        updateHover(settingsBtnSprite, { 60.0f, 60.0f }, mousePos);

        // Zapobieganie konfliktom - Hover dla StartButton
        if (startBtnSprite && startBtnSprite->getGlobalBounds().contains(mousePos)) {
            // Jesli myszka jest na przycisku, niech Hover (z State.cpp) przejmie pelna kontrole
            updateHover(startBtnSprite, { 300.0f, 120.0f }, mousePos, &startText);
        }
        else {
            // Jesli myszka zjechala z przycisku, wlaczamy ciagle pulsowanie
            buttonPulse(startBtnSprite, { 300.0f, 120.0f }, &startText);
        }
    }

    void MenuState::buttonPulse(std::optional<sf::Sprite>& btnSprite, sf::Vector2f targetSizeInPixels, std::optional<sf::Text>* linkedText)
    {
        if (!btnSprite) return;

        float time = clock.getElapsedTime().asSeconds();
        float pulse = 1.0f + pulseAmplitude * std::sin(time * pulseSpeed);

        sf::Vector2f originalSize(btnSprite->getTexture().getSize());
        btnSprite->setScale({
            (targetSizeInPixels.x / originalSize.x) * pulse,
            (targetSizeInPixels.y / originalSize.y) * pulse
            });

        // Zapewniamy powrot koloru do pierwotnego, jesli gracz zjechal z przycisku, ale Hover nie zdazyl sie wycofac
        btnSprite->setColor(sf::Color(210, 210, 210));

        if (linkedText && linkedText->has_value()) {
            (*linkedText)->setScale({ pulse, pulse });
            (*linkedText)->setFillColor(sf::Color::White); // Wycofujemy zolty kolor
        }
    }

    void MenuState::render(sf::RenderWindow& window)
    {
        if (frameSprite) window.draw(*frameSprite);

        if (startBtnSprite) window.draw(*startBtnSprite);
        if (startText) window.draw(*startText);

        if (settingsBtnSprite) window.draw(*settingsBtnSprite);

        game->drawMenuCursor();
    }



    //////////////////////////////////////////
    void MenuState::refreshTexts()
    {
        if (startText) {
            startText->setString(LocUTF8("ui_start"));
            sf::FloatRect bounds = startText->getLocalBounds();
            startText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
        }
    }
}