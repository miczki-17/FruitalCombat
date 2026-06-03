#include "MapSelectState.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h"
#include "../core/AudioManager.h"
#include "../core/LocalizationManager.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

namespace game::states
{
    using namespace game::core;

    MapSelectState::MapSelectState(game::Game* game)
        : State(game), targetIndex(0), currentScroll(0.0f)
    {
        initFireflies();
        loadRoster();
        initUI();
    }

    void MapSelectState::initUI()
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
        mapNameText.emplace(game->mainFont, "", static_cast<int>(35 * GLOBAL_FONT_SCALE));
        mapNameText->setFillColor(sf::Color::White);
        mapNameText->setOutlineColor(sf::Color::Black);
        mapNameText->setOutlineThickness(5.5f);

        mapDescText.emplace(game->mainFont, "", static_cast<int>(15 * GLOBAL_FONT_SCALE));
        mapDescText->setFillColor(sf::Color(255, 210, 120));
        mapDescText->setOutlineColor(sf::Color::Black);
        mapDescText->setOutlineThickness(2.5f);

        mapStatsText.emplace(game->mainFont, "", static_cast<int>(20 * GLOBAL_FONT_SCALE));
        mapStatsText->setFillColor(sf::Color(210, 210, 210));
        mapStatsText->setOutlineColor(sf::Color::Black);
        mapStatsText->setOutlineThickness(2.5f);

        // Przyciski i teksty
        setupButton("ui_left_arrow", leftArrowSprite, { centerX - 550.f, centerY }, { 80.f, 80.f });
        setupButton("ui_right_arrow", rightArrowSprite, { centerX + 550.f, centerY }, { 80.f, 80.f });

        setupButton("ui_empty_button", selectBtnSprite, { centerX, viewSize.y - 100.f }, { 200.f, 70.f });
        setupButtonText(selectBtnText, LocUTF8("ui_select"), { centerX, viewSize.y - 100.f - 5.f }, 27);
        selectBtnText->setOutlineColor(sf::Color::Black);
        selectBtnText->setOutlineThickness(3.2f);

        setupButton("ui_back", backBtnSprite, { 60.f, 60.f }, { 60.f, 60.f });

        setupButton("ui_star_full_icon", starFullSprite, { 0.f, 0.f }, { 25.f, 25.f });
        setupButton("ui_star_empty_icon", starEmptySprite, { 0.f, 0.f }, { 25.f, 25.f });
    }

    void MapSelectState::initFireflies()
    {
        fireflies.resize(6);
        std::random_device rd;
        std::mt19937 generator(rd());

        for (auto& f : fireflies) {
            f.lifetime = std::uniform_real_distribution<float>(0.0f, 3.0f)(generator);
            f.maxLifetime = 0.0f;
        }
    }

    void MapSelectState::updateFireflies(float dt)
    {
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float animTime = animationClock.getElapsedTime().asSeconds();
        float mapY = viewSize.y / 2.0f;
        float mapX = viewSize.x / 2.0f;

        std::random_device rd;
        std::mt19937 generator(rd());

        for (auto& f : fireflies) {
            f.lifetime += dt;

            if (f.lifetime >= f.maxLifetime) {
                f.lifetime = 0.f;
                f.position.x = mapX + std::uniform_real_distribution<float>(-200.f, 200.f)(generator);
                f.position.y = mapY + std::uniform_real_distribution<float>(-50.f, 150.f)(generator);
                f.speed = std::uniform_real_distribution<float>(15.f, 35.f)(generator);
                f.maxLifetime = std::uniform_real_distribution<float>(2.0f, 4.0f)(generator);
                f.size = std::uniform_real_distribution<float>(1.5f, 3.5f)(generator);
                f.swayOffset = std::uniform_real_distribution<float>(0.0f, 6.28f)(generator);
            }

            f.position.y -= f.speed * dt;
            f.position.x += std::sin(animTime * 2.0f + f.swayOffset) * 12.0f * dt;
            float lifeRatio = f.lifetime / f.maxLifetime;
            f.alpha = std::sin(lifeRatio * 3.14159f) * 200.f;
        }
    }

    void MapSelectState::loadRoster()
    {
        roster.clear();
        auto& rm = ResourceManager::get();

        for (auto& el : game->mapsConfig.items()) {
            std::string jsonKey = el.key();
            const auto& mapData = el.value();

            MapOption opt;
            opt.jsonKey = jsonKey;

            // Statystyki pobieramy klasycznie z konfiguracji mapy
            opt.difficultyStars = mapData.value("difficultyStars", 1);
            opt.damageMultiplier = mapData.value("damageMultiplier", 1.0f);

            roster.push_back(std::move(opt));
            auto& savedMap = roster.back();

            std::string thumbKey = jsonKey + "_thumb";
            if (rm.hasTexture(thumbKey)) {
                savedMap.thumbnailSprite.emplace(*rm.getTexture(thumbKey));
                sf::Vector2u size = savedMap.thumbnailSprite->getTexture().getSize();
                savedMap.thumbnailSprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
            }
        }

        for (int i = 0; i < roster.size(); ++i) {
            if (roster[i].jsonKey == game->selectedMapKey) {
                targetIndex = i;
                currentScroll = static_cast<float>(i);
                break;
            }
        }
    }

    void MapSelectState::handleEvent(const sf::Event& event)
    {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
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
                    int actualIndex = (targetIndex % N + N) % N;
                    game->selectedMapKey = roster[actualIndex].jsonKey;
                    game->getStateMachine().changeState(StateType::Lobby);
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                game->playUIClick();
                game->getStateMachine().changeState(StateType::Lobby);
            }
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
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
                        int actualIndex = (targetIndex % N + N) % N;
                        game->selectedMapKey = roster[actualIndex].jsonKey;
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

    void MapSelectState::update(float dt)
    {
        if (roster.empty()) return;

        currentScroll += (targetIndex - currentScroll) * 12.0f * dt;
        updateFireflies(dt);

        float N = static_cast<float>(roster.size());
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        for (int i = 0; i < roster.size(); ++i) {
            float diff = static_cast<float>(i) - currentScroll;
            while (diff < -N / 2.0f) diff += N;
            while (diff > N / 2.0f) diff -= N;
            float dist = diff;

            float xPos = centerX + dist * 500.0f;
            float scale = std::max(0.6f, 1.2f - std::abs(dist) * 0.4f);
            float alpha = std::max(0.0f, 255.0f - std::abs(dist) * 140.0f);
            float colorTint = std::max(80.0f, 255.0f - std::abs(dist) * 120.0f);

            if (roster[i].thumbnailSprite) {
                roster[i].thumbnailSprite->setPosition({ xPos, centerY });
                roster[i].thumbnailSprite->setScale({ scale, scale });
                roster[i].thumbnailSprite->setColor(sf::Color(colorTint, colorTint, colorTint, static_cast<std::uint8_t>(alpha)));
            }
        }

        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()));
        updateHover(leftArrowSprite, { 80.f, 80.f }, mousePos);
        updateHover(rightArrowSprite, { 80.f, 80.f }, mousePos);
        updateHover(selectBtnSprite, { 200.f, 70.f }, mousePos, &selectBtnText);
        updateHover(backBtnSprite, { 60.f, 60.f }, mousePos);

        int actualIndex = (targetIndex % (int)N + (int)N) % (int)N;

        // --- DYNAMICZNE TRANSLACJE AREN I OPISOW MAP ---
        if (mapNameText) {
            mapNameText->setString(LocUTF8("map_" + roster[actualIndex].jsonKey + "_name"));
            sf::FloatRect bounds = mapNameText->getLocalBounds();
            mapNameText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
            mapNameText->setPosition({ std::round(centerX), 80.f });
        }
        if (mapDescText) {
            mapDescText->setString(LocUTF8("map_" + roster[actualIndex].jsonKey + "_desc"));
            sf::FloatRect bounds = mapDescText->getLocalBounds();
            mapDescText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
            mapDescText->setPosition({ std::round(centerX), 125.f });
        }
        if (mapStatsText) {
            std::string numberStr = std::format("{:.1f}x   |   ", roster[actualIndex].damageMultiplier);
            sf::String finalString = LocUTF8("ui_damage_modifier") + numberStr;
            mapStatsText->setString(finalString);
            sf::FloatRect bounds = mapStatsText->getLocalBounds();
            mapStatsText->setOrigin({ 0.f, std::round(bounds.position.y + bounds.size.y / 2.0f) });
        }
    }

    void MapSelectState::render(sf::RenderWindow& window)
    {
        if (bgSprite) window.draw(*bgSprite);
        window.draw(darkOverlay);

        int N = roster.size();
        std::vector<std::pair<float, int>> zOrder;
        for (int i = 0; i < N; ++i) {
            float dist = std::abs(std::fmod(i - currentScroll + N + N / 2.0f, N) - N / 2.0f);
            zOrder.push_back({ dist, i });
        }
        std::sort(zOrder.begin(), zOrder.end(), [](auto& a, auto& b) { return a.first > b.first; });

        for (auto& item : zOrder) {
            if (roster[item.second].thumbnailSprite) {
                sf::FloatRect bounds = roster[item.second].thumbnailSprite->getLocalBounds();
                float scale = roster[item.second].thumbnailSprite->getScale().x;

                sf::RectangleShape shadow({ bounds.size.x * scale, bounds.size.y * scale });
                shadow.setOrigin({ bounds.size.x * scale / 2.0f, bounds.size.y * scale / 2.0f });

                sf::Vector2f thumbPos = roster[item.second].thumbnailSprite->getPosition();
                shadow.setPosition({ thumbPos.x + 10.f, thumbPos.y + 25.f });

                sf::Color spriteColor = roster[item.second].thumbnailSprite->getColor();
                shadow.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(spriteColor.a * 0.5f)));

                window.draw(shadow);
                window.draw(*roster[item.second].thumbnailSprite);
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

        if (mapNameText) {
            sf::Vector2f originalPos = mapNameText->getPosition();
            mapNameText->setPosition({ originalPos.x + 4.0f, originalPos.y + 4.0f });
            mapNameText->setFillColor(sf::Color(0, 0, 0, 150));
            mapNameText->setOutlineThickness(0.0f);
            window.draw(*mapNameText);

            mapNameText->setPosition(originalPos);
            mapNameText->setFillColor(sf::Color::White);
            mapNameText->setOutlineColor(sf::Color::Black);
            mapNameText->setOutlineThickness(4.5f);
            window.draw(*mapNameText);
        }

        if (mapDescText) window.draw(*mapDescText);

        int actualIndex = (targetIndex % (int)N + (int)N) % (int)N;
        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float centerX = viewSize.x / 2.0f;
        float uiYPosition = viewSize.y - 150.f;

        float starSpacing = 28.0f;
        float starsTotalWidth = (4 * starSpacing) + 25.f;
        float textWidth = mapStatsText ? mapStatsText->getLocalBounds().size.x : 0.f;
        float totalGroupWidth = textWidth + starsTotalWidth;
        float startX = centerX - (totalGroupWidth / 2.0f);

        if (mapStatsText) {
            mapStatsText->setPosition({ std::round(startX), std::round(uiYPosition) });
            window.draw(*mapStatsText);
        }

        int difficulty = roster[actualIndex].difficultyStars;
        float starsStartX = startX + textWidth;

        for (int i = 0; i < 5; ++i) {
            auto& currentStar = (i < difficulty) ? starFullSprite : starEmptySprite;
            if (currentStar) {
                currentStar->setPosition({ std::round(starsStartX + 12.5f + (i * starSpacing)), std::round(uiYPosition) - 10 });
                window.draw(*currentStar);
            }
        }

        if (leftArrowSprite) window.draw(*leftArrowSprite);
        if (rightArrowSprite) window.draw(*rightArrowSprite);

        if (selectBtnSprite) {
            window.draw(*selectBtnSprite);
            if (selectBtnText) window.draw(*selectBtnText);
        }

        if (backBtnSprite) window.draw(*backBtnSprite);

        game->drawMenuCursor();
    }
}