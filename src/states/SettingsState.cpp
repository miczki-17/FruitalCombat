#include "SettingsState.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h"
#include "../core/LocalizationManager.h"
#include <format>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace game::states
{
    using namespace game::core;

    SettingsState::SettingsState(game::Game* game)
        : State(game)
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
        auto& rm = ResourceManager::get();

        if (rm.hasTexture("select_bg")) {
            if (bgTexture.loadFromImage(*rm.getImage("select_bg"))) {
                bgSprite = sf::Sprite(bgTexture);
                bgSprite->setScale({ viewSize.x / bgTexture.getSize().x, viewSize.y / bgTexture.getSize().y });
            }
        }

        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(0, 0, 0, 220)); // Mocniej przyciemnione

        // --- NAGLOWEK ---
        settingsText.emplace(game->mainFont);
        settingsText->setCharacterSize(60);
        settingsText->setFillColor(sf::Color::White);
        settingsText->setStyle(sf::Text::Bold);

        // --- LEWA SEKCJA (AUDIO & JÊZYK) ---
        float leftColX = viewSize.x * 0.25f;

        audioTitle.emplace(game->mainFont);
        audioTitle->setCharacterSize(30);
        audioTitle->setFillColor(sf::Color(200, 200, 200));

        sliderTrack.setSize({ 250.f, 8.f });
        sliderTrack.setFillColor(sf::Color(80, 80, 80));
        sliderTrack.setOrigin({ 125.f, 4.f });
        sliderTrack.setPosition({ leftColX, 300.f });

        sliderHandle.setSize({ 20.f, 30.f });
        sliderHandle.setFillColor(sf::Color::White);
        sliderHandle.setOrigin({ 10.f, 15.f });

        currentVolume = sf::Listener::getGlobalVolume();
        sliderHandle.setPosition({ leftColX - 125.f + (250.f * (currentVolume / 100.f)), 300.f });

        volumeValueText.emplace(game->mainFont);
        volumeValueText->setCharacterSize(24);
        volumeValueText->setPosition({ leftColX + 140.f, 285.f });

        // PRZELACZNIK JEZYKA
        langLabel.emplace(game->mainFont);
        langLabel->setCharacterSize(30);
        langLabel->setFillColor(sf::Color(200, 200, 200));

        langBtnText.emplace(game->mainFont);
        langBtnText->setCharacterSize(30);
        langBtnText->setFillColor(sf::Color::White);

        setupButton("ui_left_arrow", langLeftArrow, { leftColX - 100.f, 470.f }, arrowSize);
        setupButton("ui_right_arrow", langRightArrow, { leftColX + 100.f, 470.f }, arrowSize);


        // --- PRAWA SEKCJA (BINDS SCROLL) ---
        float rightColX = viewSize.x * 0.75f;

        controlsTitle.emplace(game->mainFont);
        controlsTitle->setCharacterSize(30);
        controlsTitle->setFillColor(sf::Color(200, 200, 200));

        // Tlo pod liste bindow
        bindsBackground.setSize({ 450.f, 350.f });
        bindsBackground.setFillColor(sf::Color(20, 20, 20, 180));
        bindsBackground.setOutlineThickness(2.0f);
        bindsBackground.setOutlineColor(sf::Color(100, 100, 100));
        bindsBackground.setOrigin({ 200.f, 0.f });
        bindsBackground.setPosition({ rightColX, 200.f });

        // Ustawienie "kamery" (View) do scrolowania listy.
        // Viewport to procentowe wymiary ekranu (od 0.0 do 1.0)
        sf::FloatRect bgBounds = bindsBackground.getGlobalBounds();
        scrollView.setSize({ bgBounds.size.x, bgBounds.size.y });
        scrollView.setCenter({ rightColX, 200.f + (bgBounds.size.y / 2.0f) });

        float vX = bgBounds.position.x / viewSize.x;
        float vY = bgBounds.position.y / viewSize.y;
        float vW = bgBounds.size.x / viewSize.x;
        float vH = bgBounds.size.y / viewSize.y;
        scrollView.setViewport(sf::FloatRect({ vX, vY }, { vW, vH }));

        float startY = 240.f;
        float spacing = 70.f;
        setupBindRow(upLabel, upBtnText, startY);
        setupBindRow(leftLabel, leftBtnText, startY + spacing);
        setupBindRow(downLabel, downBtnText, startY + spacing * 2);
        setupBindRow(rightLabel, rightBtnText, startY + spacing * 3);

        maxScroll = 0.0f; // change if more binds

        // --- BACK BUTTON ---
        if (rm.hasTexture("ui_back")) {
            backBtnSprite.emplace(*rm.getTexture("ui_back"));
            auto size = backBtnSprite->getTexture().getSize();
            baseBackScale = 60.f / size.x;
            backBtnSprite->setScale({ baseBackScale, baseBackScale });
            backBtnSprite->setOrigin({ size.x / 2.f, size.y / 2.f });
            backBtnSprite->setPosition({ 60.f, 60.f });
        }

        refreshTexts(); // set all text as actual
    }

    void SettingsState::setupBindRow(std::optional<sf::Text>& label,
        std::optional<sf::Text>& btn,
        float yPos)
    {
        float rightColX = game->getWindow().getDefaultView().getSize().x * 0.75f;

        label.emplace(game->mainFont);
        label->setCharacterSize(24);
        label->setFillColor(sf::Color::White);

        btn.emplace(game->mainFont);
        btn->setCharacterSize(24);
        btn->setFillColor(sf::Color::Yellow);

        label->setPosition({ 0.f, yPos });
        btn->setPosition({ 0.f, yPos });
    }

    void SettingsState::refreshTexts()
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();

        std::string rawStr = Loc("ui_settings");
        settingsText->setString(sf::String::fromUtf8(rawStr.begin(), rawStr.end()));
        sf::FloatRect sBounds = settingsText->getLocalBounds();
        settingsText->setOrigin({ std::round(sBounds.size.x / 2.f), std::round(sBounds.size.y / 2.f) });
        settingsText->setPosition({ viewSize.x / 2.f, 70.f });

        // Lewa Kolumna
        float leftColX = viewSize.x * 0.25f;
        rawStr = Loc("ui_audio");
        audioTitle->setString(sf::String::fromUtf8(rawStr.begin(), rawStr.end()));
        sf::FloatRect aBounds = audioTitle->getLocalBounds();
        audioTitle->setOrigin({ std::round(aBounds.size.x / 2.f), 0.f });
        audioTitle->setPosition({ leftColX, 230.f });

        rawStr = Loc("ui_language");
        langLabel->setString(sf::String::fromUtf8(rawStr.begin(), rawStr.end()));
        sf::FloatRect lBounds = langLabel->getLocalBounds();
        langLabel->setOrigin({ std::round(lBounds.size.x / 2.f), 0.f });
        langLabel->setPosition({ leftColX, 410.f });

        // --- POZYCJONOWANIE JEZYKA I STRZALEK ---
        float rowY = 470.0f;

        std::string rawLangName = LocalizationManager::get().getCurrentLanguageName();
        langBtnText->setString(sf::String::fromUtf8(rawLangName.begin(), rawLangName.end()));

        sf::FloatRect lbBounds = langBtnText->getLocalBounds();
        langBtnText->setOrigin({
            std::round(lbBounds.size.x / 2.f),
            std::round(lbBounds.position.y + lbBounds.size.y / 2.f)
            });
        langBtnText->setPosition({ leftColX, rowY });

        // --- PRECYZYJNA KONTROLA STRZALEK ---
        constexpr float languageFieldWidth = 220.f;
        constexpr float paddingX = 60.f;

        float leftArrowX = leftColX - languageFieldWidth / 2.f - paddingX;
        float rightArrowX = leftColX + languageFieldWidth / 2.f + paddingX;

        langLeftArrow->setPosition({ leftArrowX, rowY });
        langRightArrow->setPosition({ rightArrowX, rowY });

        // Prawa Kolumna
        float rightColX = viewSize.x * 0.75f;
        rawStr = Loc("ui_controls");
        controlsTitle->setString(sf::String::fromUtf8(rawStr.begin(), rawStr.end()));
        sf::FloatRect cBounds = controlsTitle->getLocalBounds();
        controlsTitle->setOrigin({ std::round(cBounds.size.x / 2.f), 0.f });
        controlsTitle->setPosition({ rightColX, 150.f });

        
        constexpr float LABEL_COLUMN_OFFSET = 320.f;
        constexpr float KEY_COLUMN_OFFSET = 360.f;

        float panelLeft = bindsBackground.getPosition().x - bindsBackground.getSize().x / 2.f;

        float labelColumnX = panelLeft + LABEL_COLUMN_OFFSET;
        float keyColumnX = panelLeft + KEY_COLUMN_OFFSET;
        

        rawStr = Loc("ui_move_up");
        upLabel->setString(sf::String::fromUtf8(rawStr.begin(), rawStr.end()));
        rawStr = Loc("ui_move_left");
        leftLabel->setString(sf::String::fromUtf8(rawStr.begin(), rawStr.end()));
        rawStr = Loc("ui_move_down");
        downLabel->setString(sf::String::fromUtf8(rawStr.begin(), rawStr.end()));
        rawStr = Loc("ui_move_right");
        rightLabel->setString(sf::String::fromUtf8(rawStr.begin(), rawStr.end()));

        alignTextRight(*upLabel);
        alignTextRight(*leftLabel);
        alignTextRight(*downLabel);
        alignTextRight(*rightLabel);

        upLabel->setPosition({ labelColumnX, upLabel->getPosition().y });
        leftLabel->setPosition({ labelColumnX, leftLabel->getPosition().y });
        downLabel->setPosition({ labelColumnX, downLabel->getPosition().y });
        rightLabel->setPosition({ labelColumnX, rightLabel->getPosition().y });
        upBtnText->setPosition({ keyColumnX, upBtnText->getPosition().y });
        leftBtnText->setPosition({ keyColumnX, leftBtnText->getPosition().y });
        downBtnText->setPosition({ keyColumnX, downBtnText->getPosition().y });
        rightBtnText->setPosition({ keyColumnX, rightBtnText->getPosition().y });
    }

    void SettingsState::setupButton(const std::string& key, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
    {
        auto& rm = ResourceManager::get();
        if (rm.hasTexture(key))
        {
            spr.emplace(*rm.getTexture(key));
            sf::Vector2f originalSize(spr->getTexture().getSize());
            float scaleX = targetSize.x / originalSize.x;
            float scaleY = targetSize.y / originalSize.y;
            spr->setScale({ scaleX, scaleY });
            spr->setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });
            spr->setPosition(pos);
        }
        else {
            std::cerr << "[MENU ERROR] cannot find " << key << " in ResourceManager\n";
        }
    }

    void SettingsState::updateHover(std::optional<sf::Sprite>& btn, sf::Vector2f targetSize, sf::Vector2f mousePos, std::optional<sf::Text>* linkedText)
    {
        if (!btn) return;

        sf::Vector2f texSize(btn->getTexture().getSize());
        float baseScaleX = targetSize.x / texSize.x;
        float baseScaleY = targetSize.y / texSize.y;

        if (btn->getGlobalBounds().contains(mousePos)) {
            btn->setColor(sf::Color(255, 255, 255));
            btn->setScale({ baseScaleX * 1.1f, baseScaleY * 1.1f });

            if (linkedText && linkedText->has_value()) {
                (*linkedText)->setFillColor(sf::Color::Yellow); // Zmiana na zolty przy hoverze
                (*linkedText)->setScale({ 1.05f, 1.05f });
            }
        }
        else {
            btn->setColor(sf::Color(210, 210, 210));
            btn->setScale({ baseScaleX, baseScaleY });

            if (linkedText && linkedText->has_value()) {
                (*linkedText)->setFillColor(sf::Color::White);
                (*linkedText)->setScale({ 1.0f, 1.0f });
            }
        }
    }

    void SettingsState::alignTextRight(sf::Text& text)
    {
        sf::FloatRect bounds = text.getLocalBounds();

        text.setOrigin({
            bounds.position.x + bounds.size.x,
            bounds.position.y
            });
    }

    void SettingsState::handleEvent(const sf::Event& event)
    {
        // 1. Zbieranie klawisza (jesli rebindujemy)
        if (currentRebind != RebindTarget::None)
        {
            if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>())
            {
                switch (currentRebind) {
                case RebindTarget::Up:    game->keyUp = keyEvent->code; break;
                case RebindTarget::Left:  game->keyLeft = keyEvent->code; break;
                case RebindTarget::Down:  game->keyDown = keyEvent->code; break;
                case RebindTarget::Right: game->keyRight = keyEvent->code; break;
                default: break;
                }
                currentRebind = RebindTarget::None;
                game->playUIClick();
            }
            return;
        }

        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                game->playUIClick();
                game->getStateMachine().popState();
                return;
            }
        }

        // 2. Przewijanie rolka myszy -> prawa lista only
        if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
            sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()));
            if (bindsBackground.getGlobalBounds().contains(mousePos)) {
                scrollOffset -= scroll->delta * 30.0f;
                scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);

                // Aktualizacja kamery widoku listy
                sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
                float rightColX = viewSize.x * 0.75f;

                constexpr float LABEL_COLUMN_OFFSET = 60.f;
                constexpr float KEY_COLUMN_OFFSET = 90.f;

                float labelColumnX = rightColX + LABEL_COLUMN_OFFSET;
                float keyColumnX = rightColX + KEY_COLUMN_OFFSET;

                scrollView.setCenter({ rightColX, 200.f + (bindsBackground.getSize().y / 2.0f) + scrollOffset });
            }
        }

        // 3. Klikniecia
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f globalMousePos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()), game->getWindow().getDefaultView());

                // Myszka w ukladzie wspolrzednych "scrollowanej listy"
                sf::Vector2f scrolledMousePos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()), scrollView);

                if (backBtnSprite && backBtnSprite->getGlobalBounds().contains(globalMousePos)) {
                    game->playUIClick();
                    game->getStateMachine().popState();
                    return;
                }

                // Zmiana jezyka klikiem w tekst LUB w lewa/prawa strzalke
                bool leftClicked = langLeftArrow && langLeftArrow->getGlobalBounds().contains(globalMousePos);
                bool rightClicked = langRightArrow && langRightArrow->getGlobalBounds().contains(globalMousePos);
                bool textClicked = langBtnText && langBtnText->getGlobalBounds().contains(globalMousePos);

                if (leftClicked) {
                    game->playUIClick();
                    LocalizationManager::get().previousLanguage();
                    refreshTexts();
                    return;
                }
                if (rightClicked || textClicked) {
                    game->playUIClick();
                    LocalizationManager::get().nextLanguage();
                    refreshTexts();
                    return;
                }

                // Slider Dzwieku
                sf::FloatRect expandedSliderArea({ sliderTrack.getPosition().x - 125.f, sliderTrack.getPosition().y - 20.f }, { 250.f, 40.f });
                if (expandedSliderArea.contains(globalMousePos)) {
                    isDraggingSlider = true;
                }

                // Klikanie w Bindy
                if (bindsBackground.getGlobalBounds().contains(globalMousePos)) {
                    if (upBtnText->getGlobalBounds().contains(scrolledMousePos))    currentRebind = RebindTarget::Up;
                    if (leftBtnText->getGlobalBounds().contains(scrolledMousePos))  currentRebind = RebindTarget::Left;
                    if (downBtnText->getGlobalBounds().contains(scrolledMousePos))  currentRebind = RebindTarget::Down;
                    if (rightBtnText->getGlobalBounds().contains(scrolledMousePos)) currentRebind = RebindTarget::Right;
                }
            }
        }

        if (event.is<sf::Event::MouseButtonReleased>()) {
            isDraggingSlider = false;
        }
    }

    void SettingsState::update(float /*dt*/)
    {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
        sf::Vector2f uiMousePos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());

        // Hover dla przycisku powrotu
        if (backBtnSprite) updateHover(backBtnSprite, { 60.f, 60.f }, uiMousePos);

        updateHover(langLeftArrow, arrowSize, uiMousePos, &langBtnText);
        updateHover(langRightArrow, arrowSize, uiMousePos, &langBtnText);

        // --- Slider Dzwieku ---
        if (isDraggingSlider) {
            float trackStart = sliderTrack.getPosition().x - 125.f;
            float newVol = ((uiMousePos.x - trackStart) / 250.f) * 100.0f;
            currentVolume = std::clamp(newVol, 0.0f, 100.0f);
            sf::Listener::setGlobalVolume(currentVolume);
        }

        float handleX = (sliderTrack.getPosition().x - 125.f) + (250.f * (currentVolume / 100.0f));
        sliderHandle.setPosition({ handleX, sliderHandle.getPosition().y });
        volumeValueText->setString(std::format("{:.0f}%", currentVolume));

        // --- Aktualizacja Tekstow Bindow ---
        upBtnText->setString(currentRebind == RebindTarget::Up ? Loc("ui_press_key") : "[ " + keyToString(game->keyUp) + " ]");
        leftBtnText->setString(currentRebind == RebindTarget::Left ? Loc("ui_press_key") : "[ " + keyToString(game->keyLeft) + " ]");
        downBtnText->setString(currentRebind == RebindTarget::Down ? Loc("ui_press_key") : "[ " + keyToString(game->keyDown) + " ]");
        rightBtnText->setString(currentRebind == RebindTarget::Right ? Loc("ui_press_key") : "[ " + keyToString(game->keyRight) + " ]");

        // Hover dla guzika jezyka
        if (langBtnText) {
            if (langBtnText->getGlobalBounds().contains(uiMousePos)) {
                langBtnText->setFillColor(sf::Color::Yellow);
                langBtnText->setScale({ 1.1f, 1.1f });
            }
            else {
                langBtnText->setFillColor(sf::Color::White);
                langBtnText->setScale({ 1.0f, 1.0f });
            }
        }
    }

    void SettingsState::render(sf::RenderWindow& window)
    {
        // 1. Rysujemy elementy z glownej kamery (UI)
        window.setView(window.getDefaultView());
        window.draw(darkOverlay);
        if (bgSprite) window.draw(*bgSprite);

        window.draw(*settingsText);

        window.draw(*audioTitle);
        window.draw(sliderTrack);
        window.draw(sliderHandle);
        window.draw(*volumeValueText);

        window.draw(*langLabel);
        window.draw(*langBtnText);

        if (langLeftArrow) window.draw(*langLeftArrow);
        if (langRightArrow) window.draw(*langRightArrow);

        window.draw(*controlsTitle);
        window.draw(bindsBackground);
        if (backBtnSprite) window.draw(*backBtnSprite);

        // 2. Rysujemy SCROLLOWANA LISTE BINDOW
        window.setView(scrollView);

        window.draw(*upLabel);   window.draw(*upBtnText);
        window.draw(*leftLabel); window.draw(*leftBtnText);
        window.draw(*downLabel); window.draw(*downBtnText);
        window.draw(*rightLabel); window.draw(*rightBtnText);

        // 3. Zawsze na koncu wracamy do domyslnej kamery, zeby kursor dzialal poprawnie
        window.setView(window.getDefaultView());
        game->drawMenuCursor();
    }

    std::string SettingsState::keyToString(sf::Keyboard::Key key)
    {
        switch (key) {
        case sf::Keyboard::Key::W: return "W";
        case sf::Keyboard::Key::A: return "A";
        case sf::Keyboard::Key::S: return "S";
        case sf::Keyboard::Key::D: return "D";
        case sf::Keyboard::Key::Up: return "Up";
        case sf::Keyboard::Key::Left: return "Left";
        case sf::Keyboard::Key::Down: return "Down";
        case sf::Keyboard::Key::Right: return "Right";
        case sf::Keyboard::Key::Space: return "Space";
        default: return "Key";
        }
    }
}