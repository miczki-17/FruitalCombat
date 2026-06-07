// --- LobbyState.cpp ---

#include "LobbyState.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h"
#include "../core/AudioManager.h"
#include "../core/LocalizationManager.h"
#include <cmath>
#include <random>
#include <iostream>

namespace game::states
{
    using namespace game::core;

    LobbyState::LobbyState(game::Game* game)
        : State(game)
    {
        initFireflies();
        initUI();
        loadSelectedCharacter();

        // language
        lastLangCode = LocalizationManager::get().getCurrentLanguageCode();

        // Ustawienia Audio
        sf::Listener::setGlobalVolume(35.0f);
        auto& audio = AudioManager::get();
        if (!audio.isMusicPlaying("bg_music")) {
            audio.playMusic("bg_music");
        }
    }

    void LobbyState::initFireflies()
    {
        fireflies.resize(10); // Zwiekszamy ilosc swietlikow na 10
        std::random_device rd;
        std::mt19937 generator(rd());

        for (auto& f : fireflies) {
            f.lifetime = std::uniform_real_distribution<float>(0.0f, 3.0f)(generator);
            f.maxLifetime = 0.0f;
        }
    }

    void LobbyState::updateFireflies(float dt)
    {
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float animTime = animationClock.getElapsedTime().asSeconds();

        // Swietliki kraza nad i dookola klody
        float platformY = (viewSize.y / 2.0f) + 100.f;
        float platformX = viewSize.x / 2.0f;

        std::random_device rd;
        std::mt19937 generator(rd());

        for (auto& f : fireflies) {
            f.lifetime += dt;
            if (f.lifetime >= f.maxLifetime) {
                f.lifetime = 0.f;
                f.position.x = platformX + std::uniform_real_distribution<float>(-160.f, 160.f)(generator);
                f.position.y = platformY + std::uniform_real_distribution<float>(-100.f, 50.f)(generator);
                f.speed = std::uniform_real_distribution<float>(15.f, 35.f)(generator);
                f.maxLifetime = std::uniform_real_distribution<float>(2.0f, 5.0f)(generator);
                f.size = std::uniform_real_distribution<float>(1.8f, 3.5f)(generator);
                f.swayOffset = std::uniform_real_distribution<float>(0.0f, 6.28f)(generator);
            }

            f.position.y -= f.speed * dt;
            f.position.x += std::sin(animTime * 2.0f + f.swayOffset) * 12.0f * dt;
            float lifeRatio = f.lifetime / f.maxLifetime;
            f.alpha = std::sin(lifeRatio * 3.14159f) * 200.f;
        }
    }

    void LobbyState::initUI()
    {
        auto& rm = ResourceManager::get();

        // --- 1. £ADOWANIE TEKSTUR DO PAMIÊCI ---
        if (!rm.hasTexture("ui_fertilizer_regular")) {
            rm.loadTexture("ui_fertilizer_regular", "assets/textures/items/fert_regular.png", game::core::AssetGroup::Global);
        }
        if (!rm.hasTexture("ui_fertilizer_medium")) {
            rm.loadTexture("ui_fertilizer_medium", "assets/textures/items/fert_medium.png", game::core::AssetGroup::Global);
        }
        if (!rm.hasTexture("ui_fertilizer_best")) {
            rm.loadTexture("ui_fertilizer_best", "assets/textures/items/fert_best.png", game::core::AssetGroup::Global);
        }

        // --- 2. TWORZENIE G£ÓWNEJ IKONY ---
        updateMainFertilizerIcon();

        mainFertilizerText.emplace(game->mainFont);
        mainFertilizerText->setCharacterSize(static_cast<int>(18 * GLOBAL_FONT_SCALE));
        mainFertilizerText->setFillColor(sf::Color::White);
        mainFertilizerText->setOutlineThickness(1.5f);
        mainFertilizerText->setOutlineColor(sf::Color::Black);
        // Pozycjonujemy obok monet, np. przesuniête w lewo
        mainFertilizerText->setPosition({ 960.f, 25.f });

        // --- Inicjalizacja Popupu ---
        fertilizerPopupBg.setSize({ 260.f, 70.f });
        fertilizerPopupBg.setFillColor(sf::Color(30, 30, 40, 230));
        fertilizerPopupBg.setOutlineThickness(2.0f);
        fertilizerPopupBg.setOutlineColor(sf::Color(100, 100, 120));

        float popupX = 700.f;
        float popupY = 80.f;
        fertilizerPopupBg.setPosition({ popupX, popupY });

        // Funkcja lambda dla wygody tworzenia ikonek w popupie
        auto setupPopupIcon = [&](std::optional<sf::Sprite>& spr, std::optional<sf::Text>& txt, const std::string& texKey, float xOffset) {
            if (rm.hasTexture(texKey)) {
                spr.emplace(*rm.getTexture(texKey));

                sf::Vector2u texSize = spr->getTexture().getSize();
                spr->setOrigin({ texSize.x / 2.0f, texSize.y / 2.0f });

                float targetSize = 45.0f;
                spr->setScale({ targetSize / texSize.x, targetSize / texSize.y });

                spr->setPosition({ popupX + xOffset, popupY + 35.f });
            }

            txt.emplace(game->mainFont);
            txt->setString("x 0");
            txt->setCharacterSize(static_cast<int>(15 * GLOBAL_FONT_SCALE));
            txt->setFillColor(sf::Color::White);
            txt->setOutlineThickness(1.5f);
            txt->setOutlineColor(sf::Color::Black);

            sf::FloatRect bounds = txt->getLocalBounds();
            txt->setOrigin({ 0.f, std::round(bounds.position.y + bounds.size.y / 2.0f) });

            txt->setPosition({ popupX + xOffset + 22.f, popupY + 35.f });
        };

        setupPopupIcon(regFertSprite, regFertText, "ui_fertilizer_regular", 30.f);
        setupPopupIcon(medFertSprite, medFertText, "ui_fertilizer_medium", 115.f);
        setupPopupIcon(bestFertSprite, bestFertText, "ui_fertilizer_best", 200.f);

        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float margin = 20.0f;

        if (rm.hasTexture("ui_select_bg")) {
            bgSprite.emplace(*rm.getTexture("ui_select_bg"));
            sf::Vector2u bgSize(bgSprite->getTexture().getSize());
            bgSprite->setScale({ viewSize.x / bgSize.x, viewSize.y / bgSize.y });
        }
        else {
            std::cerr << "not found lobby bg texture!\n";
        }

        // Przyciski
        setupButton("ui_settings", settingsBtnSprite, { 0.0f, 0.0f }, { 60.0f, 60.0f });
        if (settingsBtnSprite) {
            sf::FloatRect bounds = settingsBtnSprite->getGlobalBounds();
            settingsBtnSprite->setPosition({ viewSize.x - margin - (bounds.size.x / 2.0f), margin + (bounds.size.y / 2.0f) });
        }

        setupButton("ui_empty_button", playBtnSprite, { centerX + 200.f, viewSize.y - 100.f }, { 250.f, 90.f });
        setupButtonText(playBtnText, LocUTF8("ui_play"), { centerX + 200.f, viewSize.y - 105.f }, 35);
        playBtnText->setOutlineColor(sf::Color::Black);
        playBtnText->setOutlineThickness(4.0f);

        setupButton("ui_empty_button", mapSelectBtnSprite, { centerX - 200.f, viewSize.y - 100.f }, { 250.f, 90.f });
        setupButtonText(mapSelectBtnText, LocUTF8("ui_map"), { centerX - 200.f, viewSize.y - 105.f }, 35);
        mapSelectBtnText->setOutlineColor(sf::Color::Black);
        mapSelectBtnText->setOutlineThickness(4.0f);

        setupButton("ui_shop", shopBtnSprite, { 0.0f, 0.0f }, { 80.0f, 80.0f });
        if (shopBtnSprite) {
            sf::FloatRect bounds = shopBtnSprite->getGlobalBounds();
            shopBtnSprite->setPosition({ 0.f + margin + (bounds.size.x / 2.0f), 0.f + margin + (bounds.size.y / 2.0f) });
        }

        setupButton("ui_achievements", achievementsBtnSprite, { 0.0f, 0.0f }, { 80.0f, 80.0f });
        if (achievementsBtnSprite) {
            sf::FloatRect bounds = achievementsBtnSprite->getGlobalBounds();
            achievementsBtnSprite->setPosition({ 0.f + margin + (bounds.size.x / 2.0f) + 100.0f, 0.f + margin + (bounds.size.y / 2.0f) });
        }

        // Inicjalizacja tekstu nad postacia
        charNameText.emplace(game->mainFont, "", static_cast<int>(35 * GLOBAL_FONT_SCALE));
        charNameText->setFillColor(sf::Color::White);
        charNameText->setOutlineColor(sf::Color::Black);
        charNameText->setOutlineThickness(5.5f);


        // coins
        if (!rm.hasTexture("coin")) {
            rm.loadTexture("coin", "assets/textures/entities/drops/juice_coin.png", game::core::AssetGroup::Global);
        }

        if (rm.hasTexture("coin"))
        {
            coinSprite.emplace(*rm.getTexture("coin"));
            coinSprite->setPosition({ 1080, 35 });
            coinSprite->setScale({1.8f, 1.8f});
        }
        else {
            std::cerr << "can not load coin texzture\n";
        }

        coinText.emplace(game->mainFont);
        coinText->setCharacterSize(static_cast<int>(18 * GLOBAL_FONT_SCALE));
        coinText->setFillColor(sf::Color(255, 200, 0));
        coinText->setOutlineThickness(1.5f);
        coinText->setOutlineColor(sf::Color::Black);
        coinText->setPosition({ 1120.f, 35.f });
    }

    void LobbyState::updateMainFertilizerIcon()
    {
        auto& rm = ResourceManager::get();
        std::string texKey = "ui_fert_regular";

        if (game->profile.equippedFertilizer == game::core::FertilizerType::Medium) texKey = "ui_fert_medium";
        if (game->profile.equippedFertilizer == game::core::FertilizerType::Best) texKey = "ui_fert_best";

        if (rm.hasTexture(texKey)) {
            mainFertilizerSprite.emplace(*rm.getTexture(texKey));
            mainFertilizerSprite->setPosition({ 975.f, 25.f });
            mainFertilizerSprite->setScale({ 0.8f, 0.8f });
        }
    }

    void LobbyState::loadSelectedCharacter()
    {
        auto& rm = game::core::ResourceManager::get();
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        float platformY = centerY + 120.f;
        if (rm.hasTexture("ui_log_platform")) {
            characterPlatformSprite.emplace(*rm.getTexture("ui_log_platform"));
            sf::Vector2u logSize(characterPlatformSprite->getTexture().getSize());
            characterPlatformSprite->setOrigin({ logSize.x / 2.0f, logSize.y / 2.0f });
            characterPlatformSprite->setPosition({ centerX, platformY });
            characterPlatformSprite->setScale({ 1.5f, 1.5f });
        }

        if (game->selectedFruitKey.empty() || !game->fruitsConfig.contains(game->selectedFruitKey)) {
            game->selectedFruitKey = game->fruitsConfig.begin().key();
        }

        std::string textureKey = game->selectedFruitKey + "_idle";
        if (!rm.hasTexture(textureKey)) textureKey = game->selectedFruitKey + "_walk";
        if (!rm.hasTexture(textureKey)) textureKey = game->selectedFruitKey;

        animFrames.clear();
        isAnimated = false;
        currentFrameIndex = 0;
        animationTimer = 0.0f;

        if (rm.hasTexture(textureKey)) {
            characterSprite.emplace(*rm.getTexture(textureKey));
            sf::Vector2u size = characterSprite->getTexture().getSize();

            int framesCount = 1;

            if (game->fruitsConfig.contains(game->selectedFruitKey)) {
                if (textureKey.find("_idle") != std::string::npos) {
                    framesCount = game->fruitsConfig[game->selectedFruitKey].value("idleFrames", 1);
                }
                else if (textureKey.find("_walk") != std::string::npos) {
                    framesCount = game->fruitsConfig[game->selectedFruitKey].value("walkFrames", 1);
                }
            }

            if (framesCount <= 1 && size.x > size.y * 1.5f) {
                framesCount = size.x / size.y;
            }

            if (framesCount > 1) {
                isAnimated = true;
                int frameWidth = size.x / framesCount;
                int frameHeight = size.y;

                for (int i = 0; i < framesCount; ++i) {
                    animFrames.push_back(sf::IntRect({ i * frameWidth, 0 }, { frameWidth, frameHeight }));
                }
                characterSprite->setTextureRect(animFrames[0]);
                characterSprite->setOrigin({ frameWidth / 2.0f, frameHeight / 2.0f });
            }
            else {
                characterSprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
            }

            float characterScale = 3.0f;
            characterSprite->setScale({ characterScale, characterScale });
            float halfCharacterHeight = (characterSprite->getTextureRect().size.y / 2.0f) * characterScale;
            characterSprite->setPosition({ centerX, platformY - halfCharacterHeight - 15.f });
        }

        // Tlumaczenie imienia (JUICE)
        if (charNameText) {
            charNameText->setString(LocUTF8("char_" + game->selectedFruitKey + "_name"));
            sf::FloatRect bounds = charNameText->getLocalBounds();
            charNameText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
            // Pozycja ustalana dynamicznie w update()
        }
    }

    void LobbyState::handleEvent(const sf::Event& event)
    {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(
                    sf::Mouse::getPosition(game->getWindow()),
                    game->getWindow().getDefaultView());

                if (characterSprite && characterSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().changeState(StateType::CharacterSelect);
                    return;
                }
                if (settingsBtnSprite && settingsBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().pushState(StateType::Settings);
                    return;
                }
                if (mapSelectBtnSprite && mapSelectBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().changeState(StateType::MapSelect);
                    return;
                }
                if (playBtnSprite && playBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().changeState(StateType::Playing);
                    return;
                }
                if (shopBtnSprite && shopBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().pushState(StateType::Shop);
                    return;
                }

                // SprawdŸ klikniêcie w ikonki w popupie (jeœli otwarty)
                if (isFertilizerPopupOpen) {
                    bool clickedInside = fertilizerPopupBg.getGlobalBounds().contains(worldPos);

                    if (regFertSprite && regFertSprite->getGlobalBounds().contains(worldPos)) {
                        game->profile.equippedFertilizer = game::core::FertilizerType::Regular;
                        game->playUIClick();
                        updateMainFertilizerIcon();
                        isFertilizerPopupOpen = false;
                        return;
                    }
                    if (medFertSprite && medFertSprite->getGlobalBounds().contains(worldPos)) {
                        game->profile.equippedFertilizer = game::core::FertilizerType::Medium;
                        game->playUIClick();
                        updateMainFertilizerIcon();
                        isFertilizerPopupOpen = false;
                        return;
                    }
                    if (bestFertSprite && bestFertSprite->getGlobalBounds().contains(worldPos)) {
                        game->profile.equippedFertilizer = game::core::FertilizerType::Best;
                        game->playUIClick();
                        updateMainFertilizerIcon();
                        isFertilizerPopupOpen = false;
                        return;
                    }

                    // Jeœli klikn¹³ gdzieœ indziej poza popupem, zamknij go
                    if (!clickedInside) {
                        isFertilizerPopupOpen = false;
                    }
                }
                else
                {
                    // Klikniêcie w g³ówn¹ ikonê otwiera popup
                    if (mainFertilizerSprite && mainFertilizerSprite->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        isFertilizerPopupOpen = true;
                        return;
                    }
                }
            }
        }
    }

    void LobbyState::update(float dt)
    {
        std::string coinStr = std::to_string(game->profile.coins);
        coinText->setString(coinStr);

        std::string currentLang = game::core::LocalizationManager::get().getCurrentLanguageCode();
        if (lastLangCode != currentLang) {
            lastLangCode = currentLang;
            refreshTexts();
        }

        updateFireflies(dt);

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(
            sf::Mouse::getPosition(game->getWindow()),
            game->getWindow().getDefaultView());

        updateHover(settingsBtnSprite, { 60.0f, 60.0f }, mousePos);
        updateHover(mapSelectBtnSprite, { 250.f, 90.f }, mousePos, &mapSelectBtnText);
        updateHover(shopBtnSprite, { 80.0f, 80.0f }, mousePos);
        updateHover(achievementsBtnSprite, { 80.0f, 80.0f }, mousePos);

        // --- JUICE: Pulsowanie guzika PLAY ---
        if (playBtnSprite && playBtnSprite->getGlobalBounds().contains(mousePos)) {
            // Hover przejmuje kontrole, zeby przycisk stal sie bialy
            updateHover(playBtnSprite, { 250.f, 90.f }, mousePos, &playBtnText);
        }
        else if (playBtnSprite) {
            // Jesli Myszki nie ma na guziku, niech wibruje "biciem serca"
            float time = animationClock.getElapsedTime().asSeconds();
            float pulse = 1.0f + 0.04f * std::sin(time * 5.0f);

            sf::Vector2f originalSize(playBtnSprite->getTexture().getSize());
            playBtnSprite->setScale({ (250.f / originalSize.x) * pulse, (90.f / originalSize.y) * pulse });
            playBtnSprite->setColor(sf::Color(210, 210, 210)); // Wycofujemy hover

            if (playBtnText) {
                playBtnText->setScale({ pulse, pulse });
                playBtnText->setFillColor(sf::Color(210, 210, 210));
            }
        }

        // --- ANIMACJA POSTACI ---
        if (isAnimated && characterSprite) {
            animationTimer += dt;
            if (animationTimer >= 0.24f) {
                animationTimer -= 0.24f;
                currentFrameIndex = (currentFrameIndex + 1) % animFrames.size();
                characterSprite->setTextureRect(animFrames[currentFrameIndex]);
            }
        }

        if (characterSprite) {
            if (characterSprite->getGlobalBounds().contains(mousePos)) {
                characterSprite->setColor(sf::Color(255, 255, 255));
                characterSprite->setScale({ 3.2f, 3.2f });
            }
            else {
                characterSprite->setColor(sf::Color(230, 230, 230));
                characterSprite->setScale({ 3.0f, 3.0f });
            }
        }

        // --- JUICE: Unoszace sie imie postaci ---
        if (charNameText && characterSprite) {
            float time = animationClock.getElapsedTime().asSeconds();
            float floatOffset = std::sin(time * 2.5f) * 9.0f; // Ruch +/- 6 pikseli w gore i w dol

            float charTopY = characterSprite->getPosition().y - (characterSprite->getGlobalBounds().size.y / 2.0f);
            charNameText->setPosition({ characterSprite->getPosition().x, charTopY - 40.f + floatOffset });
        }

        if (mainFertilizerText) {
            mainFertilizerText->setString("x" + std::to_string(game->profile.getEquippedFertilizerCount()));
        }

        if (isFertilizerPopupOpen) {
            if (regFertText) regFertText->setString("x" + std::to_string(game->profile.regularFertilizerCount));
            if (medFertText) medFertText->setString("x" + std::to_string(game->profile.mediumFertilizerCount));
            if (bestFertText) bestFertText->setString("x" + std::to_string(game->profile.bestFertilizerCount));
        }
    }


    void LobbyState::render(sf::RenderWindow& window)
    {
        if (bgSprite) window.draw(*bgSprite);

        // --- JUICE: Swietliki ---
        for (const auto& f : fireflies) {
            if (f.alpha > 0.0f) {
                sf::CircleShape particle(f.size);
                particle.setOrigin({ f.size, f.size });
                particle.setPosition(f.position);
                particle.setFillColor(sf::Color(255, 235, 140, static_cast<std::uint8_t>(f.alpha)));
                window.draw(particle);
            }
        }

        if (characterPlatformSprite) window.draw(*characterPlatformSprite);

        // --- JUICE: Ciepla, magiczna Aura z tylu bohatera ---
        //if (characterPlatformSprite) {
        //    float time = animationClock.getElapsedTime().asSeconds();
        //    float auraPulse = 1.0f + 0.15f * std::sin(time * 2.0f);

        //    sf::CircleShape glow(120.f);
        //    glow.setOrigin({ 120.f, 120.f });
        //    glow.setScale({ 1.3f * auraPulse, 0.7f * auraPulse });

        //    sf::Vector2f platformPos = characterPlatformSprite->getPosition();
        //    glow.setPosition({ platformPos.x, platformPos.y - 40.f });
        //    glow.setFillColor(sf::Color(255, 210, 80, 25)); // Delikatne, zlote swiatlo
        //    window.draw(glow);
        //}

        // RYSOWANIE CIENIA
        if (characterSprite && characterPlatformSprite) {
            float frameWidth = characterSprite->getTextureRect().size.x;
            float currentScale = characterSprite->getScale().x;
            float shadowRadius = (frameWidth * currentScale) * 0.4f;

            sf::CircleShape shadow(shadowRadius);
            shadow.setOrigin({ shadowRadius, shadowRadius });
            shadow.setScale({ 1.0f, 0.25f });

            sf::Vector2f platformPos = characterPlatformSprite->getPosition();
            shadow.setPosition({ platformPos.x, platformPos.y - 12.f });
            shadow.setFillColor(sf::Color(0, 0, 0, 100));

            window.draw(shadow);
            window.draw(*characterSprite);
        }

        // Rysowanie lewitujacego imienia
        if (charNameText) {
            window.draw(*charNameText);
        }

        if (settingsBtnSprite) window.draw(*settingsBtnSprite);
        if (playBtnSprite) window.draw(*playBtnSprite);
        if (playBtnText) window.draw(*playBtnText);
        if (mapSelectBtnSprite) window.draw(*mapSelectBtnSprite);
        if (mapSelectBtnText) window.draw(*mapSelectBtnText);

        if (shopBtnSprite) window.draw(*shopBtnSprite);
        if (achievementsBtnSprite) window.draw(*achievementsBtnSprite);

        // coins
        if (coinSprite) window.draw(*coinSprite);
        if (coinText) window.draw(*coinText);

        // NAWOZY
        if (mainFertilizerSprite) window.draw(*mainFertilizerSprite);
        if (mainFertilizerText) window.draw(*mainFertilizerText);

        if (isFertilizerPopupOpen) {
            window.draw(fertilizerPopupBg);
            if (regFertSprite) window.draw(*regFertSprite);
            if (regFertText) window.draw(*regFertText);

            if (medFertSprite) window.draw(*medFertSprite);
            if (medFertText) window.draw(*medFertText);

            if (bestFertSprite) window.draw(*bestFertSprite);
            if (bestFertText) window.draw(*bestFertText);
        }

        game->drawMenuCursor();
    }

    void LobbyState::refreshTexts()
    {
        if (playBtnText) {
            playBtnText->setString(LocUTF8("ui_play"));
            sf::FloatRect bounds = playBtnText->getLocalBounds();
            playBtnText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
        }

        if (mapSelectBtnText) {
            mapSelectBtnText->setString(LocUTF8("ui_map"));
            sf::FloatRect bounds = mapSelectBtnText->getLocalBounds();
            mapSelectBtnText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
        }

        if (charNameText) {
            charNameText->setString(LocUTF8("char_" + game->selectedFruitKey + "_name"));
            sf::FloatRect bounds = charNameText->getLocalBounds();
            charNameText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
        }
    }
}