#include "CharacterSelectState.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h"
#include "../core/AudioManager.h"
#include "../core/LocalizationManager.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>

namespace
{
    constexpr float GAME_MAX_HP = 20.0f;
    constexpr float GAME_MAX_DAMAGE = 25.0f;
    constexpr float GAME_MAX_SPEED = 600.0f;
}

namespace game::states
{
    using namespace game::core;

    CharacterSelectState::CharacterSelectState(game::Game* game)
        : State(game), targetIndex(0), currentScroll(0.0f)
    {
        initFireflies();
        loadRoster();
        initUI();
    }

    void CharacterSelectState::initUI()
    {
        auto& rm = ResourceManager::get();
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        if (rm.hasTexture("ui_select_bg")) {
            bgSprite.emplace(*rm.getTexture("ui_select_bg"));
            sf::Vector2u bgSize(bgSprite->getTexture().getSize());
            bgSprite->setScale({ viewSize.x / bgSize.x, viewSize.y / bgSize.y });
        }

        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(0, 0, 0, 0));

        // Typografia
        characterNameText.emplace(game->mainFont, "", static_cast<int>(45 * GLOBAL_FONT_SCALE));
        characterNameText->setFillColor(sf::Color::White);
        characterNameText->setOutlineColor(sf::Color::Black);
        characterNameText->setOutlineThickness(5.5f);

        characterTitleText.emplace(game->mainFont, "", static_cast<int>(20 * GLOBAL_FONT_SCALE));
        characterTitleText->setFillColor(sf::Color(255, 210, 120));
        characterTitleText->setOutlineColor(sf::Color::Black);
        characterTitleText->setOutlineThickness(3.0f);

        abilitiesTextDisplay.emplace(game->mainFont, "", static_cast<int>(18 * GLOBAL_FONT_SCALE));
        abilitiesTextDisplay->setFillColor(sf::Color(210, 210, 210));
        abilitiesTextDisplay->setOutlineColor(sf::Color::Black);
        abilitiesTextDisplay->setOutlineThickness(2.5f);

        // global methods -> State.h
        setupButton("ui_left_arrow", leftArrowSprite, { centerX - 450.f, centerY - 50.f }, { 80.f, 80.f });
        setupButton("ui_right_arrow", rightArrowSprite, { centerX + 450.f, centerY - 50.f }, { 80.f, 80.f });

        setupButton("ui_empty_button", selectBtnSprite, { centerX, viewSize.y - 100.f }, { 200.f, 70.f });
        setupButtonText(selectBtnText, LocUTF8("ui_select"), { centerX, viewSize.y - 100.f - 5.0f }, 27);
        selectBtnText->setOutlineColor(sf::Color::Black);
        selectBtnText->setOutlineThickness(3.2f);

        setupButton("ui_back", backBtnSprite, { 60.f, 60.f }, { 60.f, 60.f });

        setupButton("ui_hp_icon", hpIconSprite, { 0.f, 0.f }, { 40.f, 40.f });
        setupButton("ui_dmg_icon", dmgIconSprite, { 0.f, 0.f }, { 40.f, 40.f });
        setupButton("ui_spd_icon", spdIconSprite, { 0.f, 0.f }, { 40.f, 40.f });

        if (rm.hasTexture("ui_stat_bar_frame")) {
            statBarFrameSprite.emplace(*rm.getTexture("ui_stat_bar_frame"));
            statBarFrameSprite->setScale({ 1.5f, 1.5f });
            statBarFrameSprite->setOrigin({ 0.0f, static_cast<float>(statBarFrameSprite->getTexture().getSize().y) / 2.0f });
        }

        if (rm.hasTexture("ui_stat_bar_fill")) {
            statBarFillSprite.emplace(*rm.getTexture("ui_stat_bar_fill"));
            statBarFillSprite->setScale({ 1.5f, 1.5f });
            statBarFillSprite->setOrigin({ 0.0f, static_cast<float>(statBarFillSprite->getTexture().getSize().y) / 2.0f });
        }
    }

    void CharacterSelectState::initFireflies()
    {
        fireflies.resize(5);
        std::random_device rd;
        std::mt19937 generator(rd());

        for (auto& f : fireflies) {
            f.lifetime = std::uniform_real_distribution<float>(0.0f, 3.0f)(generator);
            f.maxLifetime = 0.0f;
        }
    }

    void CharacterSelectState::updateFireflies(float dt)
    {
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float animTime = animationClock.getElapsedTime().asSeconds();
        float platformY = (viewSize.y / 2.0f - 50.f) + (20.0f * 3.5f);
        float platformX = viewSize.x / 2.0f;

        std::random_device rd;
        std::mt19937 generator(rd());

        for (auto& f : fireflies) {
            f.lifetime += dt;
            if (f.lifetime >= f.maxLifetime) {
                f.lifetime = 0.f;
                f.position.x = platformX + std::uniform_real_distribution<float>(-90.f, 90.f)(generator);
                f.position.y = platformY + std::uniform_real_distribution<float>(-10.f, 30.f)(generator);
                f.speed = std::uniform_real_distribution<float>(20.f, 40.f)(generator);
                f.maxLifetime = std::uniform_real_distribution<float>(2.0f, 5.0f)(generator);
                f.size = std::uniform_real_distribution<float>(1.7f, 3.0f)(generator);
                f.swayOffset = std::uniform_real_distribution<float>(0.0f, 6.28f)(generator);
            }

            f.position.y -= f.speed * dt;
            f.position.x += std::sin(animTime * 2.0f + f.swayOffset) * 15.0f * dt;
            float lifeRatio = f.lifetime / f.maxLifetime;
            f.alpha = std::sin(lifeRatio * 3.14159f) * 200.f;
        }
    }

    void CharacterSelectState::loadRoster()
    {
        roster.clear();
        auto& rm = ResourceManager::get();

        for (auto& el : game->fruitsConfig.items())
        {
            std::string jsonKey = el.key();
            const auto& fruitData = el.value();

            FruitOption opt;
            opt.jsonKey = jsonKey;

            opt.displayName = fruitData.value("name", "Unknown Fighter");
            opt.title = fruitData.value("title", "");
            opt.hp = fruitData.value("hp", 10);
            opt.damage = fruitData.value("damage", 10);
            opt.speed = static_cast<int>(fruitData.value("maxSpeed", 300.0));

            opt.type = game::entities::FruitType::Apple;
            if (jsonKey == "Banana") opt.type = game::entities::FruitType::Banana;
            else if (jsonKey == "Cherry") opt.type = game::entities::FruitType::Cherry;
            else if (jsonKey == "Strawberry") opt.type = game::entities::FruitType::Strawberry;
            else if (jsonKey == "Blackberry") opt.type = game::entities::FruitType::Blackberry;
            else if (jsonKey == "Orange") opt.type = game::entities::FruitType::Orange;

            roster.push_back(std::move(opt));
            auto& savedFruit = roster.back();

            if (rm.hasTexture("ui_log_platform")) {
                savedFruit.platformSprite.emplace(*rm.getTexture("ui_log_platform"));
                sf::Vector2u logSize(savedFruit.platformSprite->getTexture().getSize());
                savedFruit.platformSprite->setOrigin({ logSize.x / 2.0f, logSize.y / 2.0f });
            }

            int idleFramesCount = fruitData.value("idleFrames", 8);

            if (rm.hasTexture(jsonKey + "_idle")) {
                savedFruit.sprite.emplace(*rm.getTexture(jsonKey + "_idle"));
                sf::Vector2u size(savedFruit.sprite->getTexture().getSize());

                if (size.x > size.y * 1.5f) {
                    savedFruit.isAnimated = true;
                    int frameWidth = size.x / idleFramesCount;
                    int frameHeight = size.y;

                    for (int i = 0; i < idleFramesCount; ++i) {
                        savedFruit.animationFrames.push_back(sf::IntRect({ i * frameWidth, 0 }, { frameWidth, frameHeight }));
                    }
                    savedFruit.sprite->setTextureRect(savedFruit.animationFrames[0]);
                    savedFruit.sprite->setOrigin({ frameWidth / 2.0f, frameHeight / 2.0f });
                }
                else {
                    savedFruit.isAnimated = false;
                    savedFruit.sprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
                }
            }
        }

        for (int i = 0; i < roster.size(); ++i) {
            if (roster[i].jsonKey == game->selectedFruitKey) {
                targetIndex = i;
                currentScroll = static_cast<float>(i);
                break;
            }
        }
    }

    void CharacterSelectState::handleEvent(const sf::Event& event)
    {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::A || keyPressed->code == sf::Keyboard::Key::Left) {
                game->playUIClick(); targetIndex--;
            }
            else if (keyPressed->code == sf::Keyboard::Key::D || keyPressed->code == sf::Keyboard::Key::Right) {
                game->playUIClick(); targetIndex++;
            }
            else if (keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Space) {
                int N = roster.size();
                if (N > 0) {
                    game->playUIClick();
                    game->selectedFruitType = roster[(targetIndex % N + N) % N].type;
                    game->selectedFruitKey = roster[(targetIndex % N + N) % N].jsonKey;
                    game->getStateMachine().changeState(StateType::Lobby);
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                game->playUIClick();
                game->getStateMachine().changeState(StateType::Lobby);
            }
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()));

                if (leftArrowSprite && leftArrowSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick(); targetIndex--;
                }
                if (rightArrowSprite && rightArrowSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick(); targetIndex++;
                }
                if (selectBtnSprite && selectBtnSprite->getGlobalBounds().contains(worldPos)) {
                    int N = roster.size();
                    if (N > 0) {
                        game->playUIClick();
                        game->selectedFruitType = roster[(targetIndex % N + N) % N].type;
                        game->selectedFruitKey = roster[(targetIndex % N + N) % N].jsonKey;
                        game->getStateMachine().changeState(StateType::Lobby);
                    }
                }
                if (backBtnSprite && backBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().changeState(StateType::Lobby);
                }
            }
        }
    }

    void CharacterSelectState::update(float dt)
    {
        if (roster.empty()) return;

        currentScroll += (targetIndex - currentScroll) * 12.0f * dt;
        updateFireflies(dt);

        float rosterYOffset = 20.f;

        float N = static_cast<float>(roster.size());
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f - 50.f + rosterYOffset;

        auto& rm = ResourceManager::get();

        for (int i = 0; i < roster.size(); ++i)
        {
            float diff = static_cast<float>(i) - currentScroll;
            while (diff < -N / 2.0f) diff += N;
            while (diff > N / 2.0f) diff -= N;
            float dist = diff;

            float xPos = centerX + dist * 320.0f;
            float yPos = centerY;
            float characterScale = std::max(1.5f, 3.0f - std::abs(dist) * 0.7f);
            float platformScale = characterScale * 0.35f;

            float alpha = std::max(0.0f, 255.0f - std::abs(dist) * 120.0f);
            float colorTint = std::max(80.0f, 255.0f - std::abs(dist) * 120.0f);
            sf::Color targetColor(static_cast<std::uint8_t>(colorTint), static_cast<std::uint8_t>(colorTint), static_cast<std::uint8_t>(colorTint), static_cast<std::uint8_t>(alpha));

            float platformYOffset = yPos + (20.0f * characterScale);

            if (roster[i].platformSprite) {
                roster[i].platformSprite->setPosition({ xPos, platformYOffset });
                roster[i].platformSprite->setScale({ platformScale, platformScale });
                roster[i].platformSprite->setColor(targetColor);
            }

            if (roster[i].sprite) {
                if (roster[i].isAnimated && std::abs(dist) < 0.5f) {
                    roster[i].animationTimer += dt;
                    if (roster[i].animationTimer >= 0.24f) {
                        roster[i].animationTimer -= 0.24f;
                        roster[i].currentFrameIndex = (roster[i].currentFrameIndex + 1) % roster[i].animationFrames.size();
                        roster[i].sprite->setTextureRect(roster[i].animationFrames[roster[i].currentFrameIndex]);
                    }
                }

                float characterYOffset = platformYOffset - (48.0f * characterScale);
                roster[i].sprite->setPosition({ xPos, characterYOffset });
                roster[i].sprite->setScale({ characterScale, characterScale });
                roster[i].sprite->setColor(targetColor);
            }
        }

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()));
        updateHover(leftArrowSprite, { 80.f, 80.f }, mousePos);
        updateHover(rightArrowSprite, { 80.f, 80.f }, mousePos);
        updateHover(selectBtnSprite, { 200.f, 70.f }, mousePos, &selectBtnText);
        updateHover(backBtnSprite, { 60.f, 60.f }, mousePos);

        int actualIndex = (targetIndex % (int)N + (int)N) % (int)N;

        // --- DYNAMICZNE TRANSLACJE POSTACI I UMIEJETNOSCI ---
        if (characterNameText) {
            characterNameText->setString(LocUTF8("char_" + roster[actualIndex].jsonKey + "_name"));
            sf::FloatRect bounds = characterNameText->getLocalBounds();
            characterNameText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
            characterNameText->setPosition({ std::round(centerX), 80.f });
        }
        if (characterTitleText) {
            characterTitleText->setString(LocUTF8("char_" + roster[actualIndex].jsonKey + "_title"));
            sf::FloatRect bounds = characterTitleText->getLocalBounds();
            characterTitleText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
            characterTitleText->setPosition({ std::round(centerX), 120.f });
        }

        // --- SK£ADANIE SKILLI (BEZPIECZNE) ---
        sf::String finalAbilitiesText = LocUTF8("ui_skills_label");

        const auto& fruitData = game->fruitsConfig[roster[actualIndex].jsonKey];
        if (fruitData.contains("abilities") && fruitData["abilities"].is_array() && !fruitData["abilities"].empty()) {

            auto abilitiesArray = fruitData["abilities"];
            for (size_t i = 0; i < abilitiesArray.size(); ++i) {
                std::string skillStr = abilitiesArray[i].get<std::string>();
                std::replace(skillStr.begin(), skillStr.end(), ' ', '_');

                finalAbilitiesText += LocUTF8("ability_" + skillStr);

                // Doklejamy przecinek TYLKO wtedy, gdy to nie jest ostatni element
                if (i < abilitiesArray.size() - 1) {
                    finalAbilitiesText += ", ";
                }
            }
        }
        else {
            finalAbilitiesText += LocUTF8("ui_none");
        }

        // --- RYSOWANIE I POZYCJONOWANIE ---
        if (abilitiesTextDisplay) {
            abilitiesTextDisplay->setString(finalAbilitiesText);

            sf::FloatRect textBounds = abilitiesTextDisplay->getLocalBounds();
            abilitiesTextDisplay->setOrigin({
                textBounds.position.x + textBounds.size.x,
                textBounds.position.y + textBounds.size.y
                });

            // Ustawiamy w prawym dolnym rogu ekranu
            sf::Vector2f viewSize = game->getWindow().getView().getSize();
            abilitiesTextDisplay->setPosition({ viewSize.x - 40.f, viewSize.y - 40.f });
        }
    }

    void CharacterSelectState::drawStatBar(sf::RenderWindow& window, std::optional<sf::Sprite>& icon, int value, float gameMaxValue, sf::Vector2f pos, sf::Color barColor, const std::string& labelText)
    {
        if (icon) {
            icon->setPosition({ std::round(pos.x + 20.f), std::round(pos.y) });
            window.draw(*icon);
        }

        if (!labelText.empty()) {
            sf::Text titleText(game->mainFont, labelText, static_cast<int>(20 * GLOBAL_FONT_SCALE));
            titleText.setFillColor(sf::Color(220, 220, 220));
            titleText.setOutlineColor(sf::Color::Black);
            titleText.setOutlineThickness(2.0f);

            sf::FloatRect titleBounds = titleText.getLocalBounds();
            titleText.setOrigin({ 0.f, std::round(titleBounds.position.y + titleBounds.size.y / 2.0f) });
            titleText.setPosition({ std::round(pos.x + 50.f), std::round(pos.y) });
            window.draw(titleText);
        }

        float barStartX = pos.x + 110.f;
        sf::Vector2f barPos = { std::round(barStartX), std::round(pos.y) };

        if (statBarFrameSprite) {
            statBarFrameSprite->setPosition(barPos);
            window.draw(*statBarFrameSprite);
        }

        if (statBarFillSprite) {
            float fillFactor = std::clamp(static_cast<float>(value) / gameMaxValue, 0.0f, 1.0f);
            sf::Vector2u texSize = statBarFillSprite->getTexture().getSize();
            int visibleWidth = static_cast<int>(texSize.x * fillFactor);

            statBarFillSprite->setTextureRect(sf::IntRect({ 0, 0 }, { visibleWidth, static_cast<int>(texSize.y) }));
            statBarFillSprite->setPosition(barPos);
            statBarFillSprite->setColor(barColor);
            window.draw(*statBarFillSprite);
        }

        sf::Text valueText(game->mainFont, std::to_string(value), static_cast<int>(20 * GLOBAL_FONT_SCALE));
        valueText.setFillColor(sf::Color::White);
        valueText.setOutlineColor(sf::Color::Black);
        valueText.setOutlineThickness(2.0f);

        float barDisplayWidth = statBarFrameSprite ? statBarFrameSprite->getGlobalBounds().size.x : 100.0f;
        sf::FloatRect textBounds = valueText.getLocalBounds();
        valueText.setOrigin({ 0.f, std::round(textBounds.position.y + textBounds.size.y / 2.0f) });
        valueText.setPosition({ std::round(barStartX + barDisplayWidth + 15.f), std::round(pos.y) });
        window.draw(valueText);
    }

    void CharacterSelectState::render(sf::RenderWindow& window)
    {
        if (bgSprite) window.draw(*bgSprite);
        window.draw(darkOverlay);

        int N = roster.size();
        renderZOrder.clear();
        if (N > 0) {
            for (int i = 0; i < N; ++i) {
                float dist = std::abs(std::fmod(i - currentScroll + N + N / 2.0f, N) - N / 2.0f);
                renderZOrder.push_back({ dist, i });
            }
            std::sort(renderZOrder.begin(), renderZOrder.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
        }

        for (const auto& item : renderZOrder) {
            if (roster[item.second].platformSprite) {
                window.draw(*roster[item.second].platformSprite);
            }

            if (roster[item.second].sprite) {
                sf::CircleShape shadow(20.0f);
                shadow.setOrigin({ 20.0f, 20.0f });
                float groupScale = roster[item.second].sprite->getScale().x;
                shadow.setScale({ groupScale * 1.1f, groupScale * 0.3f });

                sf::Vector2f charPos = roster[item.second].sprite->getPosition();
                shadow.setPosition({ charPos.x, charPos.y + (23.0f * groupScale) });
                sf::Color spriteColor = roster[item.second].sprite->getColor();
                shadow.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(spriteColor.a * 0.4f)));
                window.draw(shadow);

                window.draw(*roster[item.second].sprite);
            }
        }

        for (const auto& f : fireflies) {
            if (f.alpha > 0.0f) {
                sf::CircleShape particle(f.size);
                particle.setOrigin({ f.size, f.size });
                particle.setPosition(f.position);
                particle.setFillColor(sf::Color(255, 235, 140, static_cast<std::uint8_t>(f.alpha)));
                window.draw(particle);
            }
        }

        if (!roster.empty()) {
            int actualIndex = (targetIndex % N + N) % N;
            sf::Vector2f viewSize = game->getWindow().getView().getSize();

            float barStartX = 45.f;
            float baseBarY = viewSize.y - 180.f;
            float spacingY = 45.f;

            sf::RectangleShape statsPanelBg({ 420.f, 160.f });
            statsPanelBg.setPosition({ 20.f, viewSize.y - 210.f });
            statsPanelBg.setFillColor(sf::Color(20, 20, 25, 210));
            statsPanelBg.setOutlineThickness(2.f);
            statsPanelBg.setOutlineColor(sf::Color(80, 80, 80, 150));
            window.draw(statsPanelBg);

            drawStatBar(window, hpIconSprite, roster[actualIndex].hp, GAME_MAX_HP, { barStartX, baseBarY }, sf::Color(240, 50, 75), "HP");
            drawStatBar(window, dmgIconSprite, roster[actualIndex].damage, GAME_MAX_DAMAGE, { barStartX, baseBarY + spacingY }, sf::Color(255, 175, 0), "DMG");
            drawStatBar(window, spdIconSprite, roster[actualIndex].speed, GAME_MAX_SPEED, { barStartX, baseBarY + spacingY * 2 }, sf::Color(40, 200, 255), "SPD");

            if (abilitiesTextDisplay) {
                //abilitiesTextDisplay->setString(roster[actualIndex].abilitiesText);
                sf::FloatRect textBounds = abilitiesTextDisplay->getLocalBounds();
                abilitiesTextDisplay->setOrigin({ textBounds.size.x, textBounds.position.y + textBounds.size.y });
                abilitiesTextDisplay->setPosition({ viewSize.x - 40.f, viewSize.y - 40.f });
                window.draw(*abilitiesTextDisplay);
            }
        }

        if (characterNameText) {
            sf::Vector2f originalPos = characterNameText->getPosition();
            characterNameText->setPosition({ originalPos.x + 4.0f, originalPos.y + 4.0f });
            characterNameText->setFillColor(sf::Color(0, 0, 0, 150));
            characterNameText->setOutlineThickness(0.0f);
            window.draw(*characterNameText);

            characterNameText->setPosition(originalPos);
            characterNameText->setFillColor(sf::Color::White);
            characterNameText->setOutlineColor(sf::Color::Black);
            characterNameText->setOutlineThickness(4.5f);
            window.draw(*characterNameText);
        }

        if (characterTitleText) window.draw(*characterTitleText);

        if (leftArrowSprite) window.draw(*leftArrowSprite);
        if (rightArrowSprite) window.draw(*rightArrowSprite);
        if (selectBtnSprite) window.draw(*selectBtnSprite);
        if (selectBtnText) window.draw(*selectBtnText);
        if (backBtnSprite) window.draw(*backBtnSprite);

        game->drawMenuCursor();
    }
}