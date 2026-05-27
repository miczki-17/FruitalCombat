// ==========================================
// CharacterSelectState.cpp
// ==========================================
#include "CharacterSelectState.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h" // NOWE! Odwołanie do menedżera
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>

namespace
{
    // stałe do pasków statystyk
    constexpr float GAME_MAX_HP = 20.0f;
    constexpr float GAME_MAX_DAMAGE = 25.0f;
    constexpr float GAME_MAX_SPEED = 600.0f;
}

namespace game::states
{
    CharacterSelectState::CharacterSelectState(game::Game* game)
        : State(game), targetIndex(0), currentScroll(0.0f)
    {
        auto& rm = game::core::ResourceManager::get();

        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        if (rm.hasTexture("ui_select_bg"))
        {
            bgSprite.emplace(*rm.getTexture("ui_select_bg"));
            sf::Vector2u bgSize(bgSprite->getTexture().getSize());
            bgSprite->setScale({ viewSize.x / bgSize.x, viewSize.y / bgSize.y });
        }

        // dark overlay
        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(0, 0, 0, 0));

        characterNameText.emplace(game->mainFont, "", 45);
        characterNameText->setFillColor(sf::Color(255, 255, 255));
        characterNameText->setOutlineColor(sf::Color::Black);
        characterNameText->setOutlineThickness(5.5f);

        characterTitleText.emplace(game->mainFont, "", 20);
        characterTitleText->setFillColor(sf::Color(255, 210, 120));
        characterTitleText->setOutlineColor(sf::Color::Black);
        characterTitleText->setOutlineThickness(3.0f);

        abilitiesTextDisplay.emplace(game->mainFont, "", 18);
        abilitiesTextDisplay->setFillColor(sf::Color(210, 210, 210));
        abilitiesTextDisplay->setOutlineColor(sf::Color::Black);
        abilitiesTextDisplay->setOutlineThickness(2.5f);

        initFireflies();
        loadRoster();

        // 2. UI elements
        setupButton("ui_left_arrow", leftArrowTex, leftArrowSprite, { centerX - 450.f, centerY - 50.f }, { 80.f, 80.f });
        setupButton("ui_right_arrow", rightArrowTex, rightArrowSprite, { centerX + 450.f, centerY - 50.f }, { 80.f, 80.f });
        setupButton("ui_empty_button", selectBtnTex, selectBtnSprite, { centerX, viewSize.y - 100.f }, { 170.f, 70.f });

        // Setup Text for Select Button
        selectBtnText.emplace(game->mainFont, "SELECT", 27);
        selectBtnText->setFillColor(sf::Color(255, 255, 255));
        selectBtnText->setOutlineColor(sf::Color::Black);
        selectBtnText->setOutlineThickness(4.5f);

        sf::FloatRect chooseTextRect = selectBtnText->getLocalBounds();
        selectBtnText->setOrigin({ std::round(chooseTextRect.position.x + chooseTextRect.size.x / 2.0f),
                                   std::round(chooseTextRect.position.y + chooseTextRect.size.y / 2.0f) });
        selectBtnText->setPosition({ centerX, viewSize.y - 100.f });

        // Additional UI
        setupButton("ui_back", backBtnTex, backBtnSprite, { 50.f, 50.f }, { 60.f, 60.f });
        setupButton("ui_hp_icon", hpIconTex, hpIconSprite, { 0.f, 0.f }, { 40.f, 40.f });
        setupButton("ui_dmg_icon", dmgIconTex, dmgIconSprite, { 0.f, 0.f }, { 40.f, 40.f });
        setupButton("ui_spd_icon", spdIconTex, spdIconSprite, { 0.f, 0.f }, { 40.f, 40.f });

        // --- Statistics bars ---
        if (rm.hasTexture("ui_stat_bar_frame"))
        {
            statBarFrameSprite.emplace(*rm.getTexture("ui_stat_bar_frame"));
            statBarFrameSprite->setScale({ 1.5f, 1.5f });
            statBarFrameSprite->setOrigin({ 0.0f, static_cast<float>(statBarFrameSprite->getTexture().getSize().y) / 2.0f });
        }

        if (rm.hasTexture("ui_stat_bar_fill"))
        {
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

    // --- ROASTER ---
    void CharacterSelectState::loadRoster()
    {
        roster.clear();
        auto& rm = game::core::ResourceManager::get();

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

            opt.abilitiesText = "Skills: ";
            if (fruitData.contains("abilities") && fruitData["abilities"].is_array() && !fruitData["abilities"].empty()) {
                for (const auto& skill : fruitData["abilities"]) {
                    opt.abilitiesText += skill.get<std::string>() + ", ";
                }
                opt.abilitiesText.pop_back();
                opt.abilitiesText.pop_back();
            }
            else {
                opt.abilitiesText = "Skills: None";
            }

            opt.type = game::entities::FruitType::Apple;
            if (jsonKey == "Banana") opt.type = game::entities::FruitType::Banana;
            else if (jsonKey == "Cherry") opt.type = game::entities::FruitType::Cherry;
            else if (jsonKey == "Strawberry") opt.type = game::entities::FruitType::Strawberry;
            else if (jsonKey == "Blackberry") opt.type = game::entities::FruitType::Blackberry;
            else if (jsonKey == "Orange") opt.type = game::entities::FruitType::Orange;

            roster.push_back(std::move(opt));
            auto& savedFruit = roster.back();

            
            if (rm.hasTexture("ui_log_platform"))
            {
                savedFruit.platformSprite.emplace(*rm.getTexture("ui_log_platform"));
                sf::Vector2u logSize(savedFruit.platformSprite->getTexture().getSize());
                savedFruit.platformSprite->setOrigin({ logSize.x / 2.0f, logSize.y / 2.0f });
            }

            int idleFramesCount = fruitData.value("idleFrames", 8);
            int initFramesCount = fruitData.value("initFrames", 8);

            if (rm.hasTexture(jsonKey + "_idle"))
            {
                savedFruit.sprite.emplace(*rm.getTexture(jsonKey + "_idle"));
                sf::Vector2u size(savedFruit.sprite->getTexture().getSize());

                if (size.x > size.y * 1.5f)
                {
                    savedFruit.isAnimated = true;
                    int frameWidth = size.x / idleFramesCount;
                    int frameHeight = size.y;

                    for (int i = 0; i < idleFramesCount; ++i) {
                        savedFruit.animationFrames.push_back(sf::IntRect({ i * frameWidth, 0 }, { frameWidth, frameHeight }));
                    }
                    savedFruit.sprite->setTextureRect(savedFruit.animationFrames[0]);
                    savedFruit.sprite->setOrigin({ frameWidth / 2.0f, frameHeight / 2.0f });
                }
                else
                {
                    savedFruit.isAnimated = false;
                    savedFruit.sprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
                }
            }

            if (rm.hasTexture(jsonKey + "_start"))
            {
                savedFruit.hasStartAnimation = true;


                sf::Vector2u size(rm.getTexture(jsonKey + "_start")->getSize());
                int frameWidth = size.x / initFramesCount;
                int frameHeight = size.y;

                for (int i = 0; i < initFramesCount; ++i) {
                    savedFruit.startAnimationFrames.push_back(sf::IntRect({ i * frameWidth, 0 }, { frameWidth, frameHeight }));
                }
            }
        }
    }

    void CharacterSelectState::setupButton(const std::string& key, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
    {
        auto& rm = game::core::ResourceManager::get();
        if (rm.hasTexture(key))
        {
            spr.emplace(*rm.getTexture(key));
            sf::Vector2u originalSize(spr->getTexture().getSize());
            spr->setScale({ targetSize.x / originalSize.x, targetSize.y / originalSize.y });
            spr->setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });
            spr->setPosition(pos);
        }
    }

    StateType CharacterSelectState::getType() const { return StateType::CharacterSelect; }

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
                game->playUIClick();
                int N = roster.size();
                if (N > 0) {
                    game->selectedFruitType = roster[(targetIndex % N + N) % N].type;
                    game->getStateMachine().changeState(StateType::MapSelect);
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                game->playUIClick();
                game->getStateMachine().changeState(StateType::Menu);
            }
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);

                if (leftArrowSprite && leftArrowSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick(); targetIndex--;
                }
                if (rightArrowSprite && rightArrowSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick(); targetIndex++;
                }
                if (selectBtnSprite && selectBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    int N = roster.size();
                    if (N > 0) {
                        game->selectedFruitType = roster[(targetIndex % N + N) % N].type;
                        game->getStateMachine().changeState(StateType::MapSelect);
                    }
                }
                if (backBtnSprite && backBtnSprite->getGlobalBounds().contains(worldPos)) {
                    game->playUIClick();
                    game->getStateMachine().changeState(StateType::Menu);
                }
            }
        }
    }

    void CharacterSelectState::update(float dt)
    {
        if (roster.empty()) return;

        currentScroll += (targetIndex - currentScroll) * 12.0f * dt;
        updateFireflies(dt);

        float N = static_cast<float>(roster.size());
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f - 50.f;

        auto& rm = game::core::ResourceManager::get();

        for (int i = 0; i < roster.size(); ++i)
        {
            float diff = static_cast<float>(i) - currentScroll;
            while (diff < -N / 2.0f) diff += N;
            while (diff > N / 2.0f) diff -= N;
            float dist = diff;

            float xPos = centerX + dist * 320.0f;
            float yPos = centerY;

            float characterScale = std::max(1.5f, 3.5f - std::abs(dist) * 0.8f);
            float platformScale = characterScale * 0.35f;

            float alpha = std::max(0.0f, 255.0f - std::abs(dist) * 120.0f);
            float colorTint = std::max(80.0f, 255.0f - std::abs(dist) * 120.0f);
            sf::Color targetColor(
                static_cast<std::uint8_t>(colorTint),
                static_cast<std::uint8_t>(colorTint),
                static_cast<std::uint8_t>(colorTint),
                static_cast<std::uint8_t>(alpha)
            );

            float platformYOffset = yPos + (20.0f * characterScale);

            if (roster[i].platformSprite)
            {
                roster[i].platformSprite->setPosition({ xPos, platformYOffset });
                roster[i].platformSprite->setScale({ platformScale, platformScale });
                roster[i].platformSprite->setColor(targetColor);
            }

            if (roster[i].sprite)
            {
                if (roster[i].isAnimated && std::abs(dist) < 0.5f)
                {
                    if (roster[i].hasStartAnimation && !roster[i].hasPlayedStartAnimation)
                    {
                        roster[i].isPlayingStartAnimation = true;
                        roster[i].hasPlayedStartAnimation = true;
                        roster[i].currentFrameIndex = 0;
                        roster[i].animationTimer = 0.0f;

                        if (rm.hasTexture(roster[i].jsonKey + "_start")) {
                            roster[i].sprite->setTexture(*rm.getTexture(roster[i].jsonKey + "_start"), true);
                        }

                        if (!roster[i].startAnimationFrames.empty()) {
                            roster[i].sprite->setTextureRect(roster[i].startAnimationFrames[0]);
                        }
                    }

                    roster[i].animationTimer += dt;
                    if (roster[i].animationTimer >= 0.24f)
                    {
                        roster[i].animationTimer -= 0.24f;

                        if (roster[i].isPlayingStartAnimation)
                        {
                            roster[i].currentFrameIndex++;

                            if (roster[i].currentFrameIndex >= roster[i].startAnimationFrames.size()) {
                                roster[i].isPlayingStartAnimation = false;
                                roster[i].currentFrameIndex = 0;

                                if (rm.hasTexture(roster[i].jsonKey + "_idle")) {
                                    roster[i].sprite->setTexture(*rm.getTexture(roster[i].jsonKey + "_idle"), true);
                                }

                                if (!roster[i].animationFrames.empty()) {
                                    roster[i].sprite->setTextureRect(roster[i].animationFrames[0]);
                                }
                            }
                            else {
                                roster[i].sprite->setTextureRect(roster[i].startAnimationFrames[roster[i].currentFrameIndex]);
                            }
                        }
                        else
                        {
                            roster[i].currentFrameIndex = (roster[i].currentFrameIndex + 1) % roster[i].animationFrames.size();

                            if (rm.hasTexture(roster[i].jsonKey + "_idle")) {
                                roster[i].sprite->setTexture(*rm.getTexture(roster[i].jsonKey + "_idle"), true);
                            }

                            roster[i].sprite->setTextureRect(roster[i].animationFrames[roster[i].currentFrameIndex]);
                        }
                    }
                }
                else if (roster[i].isAnimated)
                {
                    roster[i].animationTimer = 0.0f;
                    roster[i].currentFrameIndex = 0;
                    roster[i].isPlayingStartAnimation = false;
                    roster[i].hasPlayedStartAnimation = false;

                    if (rm.hasTexture(roster[i].jsonKey + "_idle")) {
                        roster[i].sprite->setTexture(*rm.getTexture(roster[i].jsonKey + "_idle"), true);
                    }
                    if (!roster[i].animationFrames.empty()) {
                        roster[i].sprite->setTextureRect(roster[i].animationFrames[0]);
                    }
                }

                float characterYOffset = platformYOffset - (48.0f * characterScale);
                roster[i].sprite->setPosition({ xPos, characterYOffset });
                roster[i].sprite->setScale({ characterScale, characterScale });
                roster[i].sprite->setColor(targetColor);
            }
        }

        sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
        sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);

        auto updateHover = [&](std::optional<sf::Sprite>& btn, sf::Vector2f targetSize, std::optional<sf::Text>* txt = nullptr) {
            if (!btn) return;
            sf::Vector2u texSize(btn->getTexture().getSize());
            float baseScaleX = targetSize.x / texSize.x;
            float baseScaleY = targetSize.y / texSize.y;

            if (btn->getGlobalBounds().contains(worldPos)) {
                btn->setColor(sf::Color(255, 255, 255));
                btn->setScale({ baseScaleX * 1.08f, baseScaleY * 1.08f });

                if (txt && txt->has_value()) {
                    (*txt)->setFillColor(sf::Color(255, 255, 255));
                    (*txt)->setScale({ 1.08f, 1.08f });
                }
            }
            else {
                btn->setColor(sf::Color(210, 210, 210));
                btn->setScale({ baseScaleX, baseScaleY });

                if (txt && txt->has_value()) {
                    (*txt)->setFillColor(sf::Color(210, 210, 210));
                    (*txt)->setScale({ 1.0f, 1.0f });
                }
            }
            };

        updateHover(leftArrowSprite, { 80.f, 80.f });
        updateHover(rightArrowSprite, { 80.f, 80.f });
        updateHover(selectBtnSprite, { 170.f, 70.f }, &selectBtnText);
        updateHover(backBtnSprite, { 60.f, 60.f });

        int actualIndex = (targetIndex % (int)N + (int)N) % (int)N;

        if (characterNameText) {
            characterNameText->setString(roster[actualIndex].displayName);
            sf::FloatRect bounds = characterNameText->getLocalBounds();
            characterNameText->setOrigin({ std::round(bounds.position.x + bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
            characterNameText->setPosition({ std::round(centerX), 80.f });
        }
        if (characterTitleText) {
            characterTitleText->setString(roster[actualIndex].title);
            sf::FloatRect bounds = characterTitleText->getLocalBounds();
            characterTitleText->setOrigin({ std::round(bounds.position.x + bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
            characterTitleText->setPosition({ std::round(centerX), 120.f });
        }
    }

    void CharacterSelectState::drawStatBar(sf::RenderWindow& window, std::optional<sf::Sprite>& icon, int value, float gameMaxValue, sf::Vector2f pos, sf::Color barColor, const std::string& labelText)
    {
        if (icon) {
            icon->setPosition({ std::round(pos.x + 20.f), std::round(pos.y) });
            window.draw(*icon);
        }

        if (!labelText.empty()) {
            sf::Text titleText(game->mainFont, labelText, 20);
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

        sf::Text valueText(game->mainFont, std::to_string(value), 20);
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
            }

            if (roster[item.second].sprite) {
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

            drawStatBar(window, hpIconSprite, roster[actualIndex].hp, GAME_MAX_HP,
                { barStartX, baseBarY }, sf::Color(240, 50, 75), "HP");

            drawStatBar(window, dmgIconSprite, roster[actualIndex].damage, GAME_MAX_DAMAGE,
                { barStartX, baseBarY + spacingY }, sf::Color(255, 175, 0), "DMG");

            drawStatBar(window, spdIconSprite, roster[actualIndex].speed, GAME_MAX_SPEED,
                { barStartX, baseBarY + spacingY * 2 }, sf::Color(40, 200, 255), "SPD");

            if (abilitiesTextDisplay) {
                abilitiesTextDisplay->setString(roster[actualIndex].abilitiesText);

                sf::FloatRect textBounds = abilitiesTextDisplay->getLocalBounds();

                abilitiesTextDisplay->setOrigin({ textBounds.position.x + textBounds.size.x,
                                                  textBounds.position.y + textBounds.size.y });

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
            characterNameText->setFillColor(sf::Color(255, 255, 255));
            characterNameText->setOutlineColor(sf::Color::Black);
            characterNameText->setOutlineThickness(4.5f);
            window.draw(*characterNameText);
        }

        if (characterTitleText) window.draw(*characterTitleText);

        if (leftArrowSprite) window.draw(*leftArrowSprite);
        if (rightArrowSprite) window.draw(*rightArrowSprite);

        if (selectBtnSprite) {
            window.draw(*selectBtnSprite);
            if (selectBtnText) {
                window.draw(*selectBtnText);
            }
        }

        if (backBtnSprite) window.draw(*backBtnSprite);

        game->drawMenuCursor();
    }
}