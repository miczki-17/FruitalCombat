// --- PlayingState.cpp ---

#include "PlayingState.h"
#include "../core/ResourceManager.h"
#include "../core/AudioManager.h"
#include "../core/LocalizationManager.h"
#include "../components/AbilityComponent.h"
#include "../components/StatsComponent.h"
#include "../components/TransformComponent.h"
#include "../systems/EvolutionManager.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <cmath>

namespace game::states
{
    using namespace game::core;

    PlayingState::PlayingState(game::Game* game) : State(game)
    {
        std::cout << "[PlayingState] World initialization...\n";

        // Zwalniamy RAM z zasobow MENU, INTRO
        AudioManager::get().stopMusic();
        AudioManager::get().stopAllSounds();
        ResourceManager::get().unloadGroup(AssetGroup::Menu);
        ResourceManager::get().unloadGroup(AssetGroup::Intro);

        // Ladowanie zasobow
        loadCharacterAssets();
        loadEnemyAssets();
        loadMapAssets();

        initHUD();

        // Utworzenie GameWorld'a
        sf::Vector2f startPos(mapLimits.x / 2.0f, mapLimits.y / 2.0f);
        world = std::make_unique<game::core::GameWorld>(game, collisionMask, mapScale, startPos);

        // Ustawienie kamery
        cameraView = game->getWindow().getDefaultView();
        cameraView.zoom(1.4f);
    }

    PlayingState::~PlayingState()
    {
        AudioManager::get().stopMusic();
        AudioManager::get().stopAllSounds();
        // Zwalniamy zasoby z grupy Playing.
        ResourceManager::get().unloadGroup(AssetGroup::Playing);
    }

    // --- LOADING HELPERS ---

    void PlayingState::loadCharacterAssets()
    {
        auto& rm = ResourceManager::get();
        std::string fruitKey = "";

        switch (game->selectedFruitType) {
        case game::entities::FruitType::Apple:      fruitKey = "Apple"; break;
        case game::entities::FruitType::Banana:     fruitKey = "Banana"; break;
        case game::entities::FruitType::Orange:     fruitKey = "Orange"; break;
        case game::entities::FruitType::Cherry:     fruitKey = "Cherry"; break;
        case game::entities::FruitType::Strawberry: fruitKey = "Strawberry"; break;
        case game::entities::FruitType::Blackberry: fruitKey = "Blackberry"; break;
        }

        if (game->fruitsConfig.contains(fruitKey))
        {
            const auto& fruitData = game->fruitsConfig[fruitKey];

            // Efekty "Splash" dla atakow
            if (fruitData.contains("splashKey") && fruitData.contains("splashProjectileTexturePath"))
            {
                loadedSplashKey_ = fruitData.value("splashKey", "");
                std::string basePath = fruitData.value("splashProjectileTexturePath", "");

                for (int i = 1; i <= 3; i++) {
                    std::string finalKey = loadedSplashKey_ + "_" + std::to_string(i);
                    std::string finalPath = basePath + std::to_string(i) + ".png";
                    rm.loadTexture(finalKey, finalPath, AssetGroup::Playing);
                }
            }

            // Animacja chodzenia
            if (fruitData.contains("walkTexturePath")) {
                std::string path = fruitData.value("walkTexturePath", "");
                if (!path.empty()) rm.loadTexture(fruitKey + "_walk", path, AssetGroup::Playing);
            }
        }
    }

    void PlayingState::loadEnemyAssets()
    {
        auto& rm = ResourceManager::get();
        for (auto& [enemyKey, enemyData] : game->enemiesConfig.items()) {
            if (enemyData.contains("idleTexturePath")) {
                rm.loadTexture(enemyKey + "_idle", enemyData.value("idleTexturePath", ""), AssetGroup::Playing);
            }
            if (enemyData.contains("walkTexturePath")) {
                rm.loadTexture(enemyKey + "_walk", enemyData.value("walkTexturePath", ""), AssetGroup::Playing);
            }
        }
    }

    void PlayingState::loadMapAssets()
    {
        auto& rm = ResourceManager::get();
        std::string mapKey = game->selectedMapKey;
        const auto& mapData = game->mapsConfig[mapKey];

        // 1. Tlo areny i maska kolizji
        if (mapData.contains("texturePath")) {
            rm.loadTexture(mapKey + "_map", mapData.value("texturePath", ""), AssetGroup::Playing);
        }

        std::string mapMaskPath = mapData.value("maskPath", "");

        if (rm.hasTexture(mapKey + "_map")) {
            mapSprite.emplace(*rm.getTexture(mapKey + "_map"));
            mapSprite->setScale({ mapScale, mapScale });
            sf::Vector2u rawSize = rm.getTexture(mapKey + "_map")->getSize();
            mapLimits = sf::Vector2f(rawSize.x * mapScale, rawSize.y * mapScale);
        }

        if (!collisionMask.loadFromFile(mapMaskPath)) {
            std::cerr << "[ERROR] Could not load collision mask from: " << mapMaskPath << '\n';
        }

        // 2. Specyficzne zasoby mapy
        if (mapKey == "CrisperDrawer") {
            rm.loadTexture("hazard_icicle", "assets/textures/hazards/icicle.png", AssetGroup::Playing);
            rm.loadTexture("hazard_icicle_shard", "assets/textures/hazards/icicle_shard.png", AssetGroup::Playing);
            rm.loadSound("icicle_shatter", "assets/sounds/icicle_shatter.mp3", AssetGroup::Playing);
        }
        else if (mapKey == "ChoppingBlock") {
            rm.loadTexture("hazard_knife", "assets/textures/hazards/knife.png", AssetGroup::Playing);
            rm.loadSound("knife_hit", "assets/sounds/knife_hit.mp3", AssetGroup::Playing);
        }
        else if (mapKey == "WildOrchard") {
            rm.loadTexture("hazard_spore", "assets/textures/hazards/spore.png", AssetGroup::Playing);
            rm.loadTexture("hazard_spore_splash", "assets/textures/hazards/spore_splash_1.png", AssetGroup::Playing);
            rm.loadSound("spore_splat", "assets/sounds/spore_splat.mp3", AssetGroup::Playing);
        }

        // 3. Opcje ogolne
        rm.loadTexture("magic_bean_base", "assets/textures/entities/drops/magic_bean_base.png", AssetGroup::Playing);

        game->arenaContext.mapDustColor = sf::Color(
            static_cast<std::uint8_t>(mapData.value("dustR", 150)),
            static_cast<std::uint8_t>(mapData.value("dustG", 150)),
            static_cast<std::uint8_t>(mapData.value("dustB", 150)),
            static_cast<std::uint8_t>(mapData.value("dustA", 150))
        );
    }

    // --- INITIALIZATION ---

    void PlayingState::initHUD()
    {
        auto& rm = ResourceManager::get();
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();

        // --- Pasek HP ---
        hpBarBg.setSize({ 200.f, 20.f });
        hpBarBg.setFillColor(sf::Color(40, 40, 40, 200));
        hpBarBg.setOutlineThickness(2.f);
        hpBarBg.setOutlineColor(sf::Color::Black);

        hpBarFill.setSize({ 200.f, 20.f });
        hpBarFill.setFillColor(sf::Color(220, 30, 30));

        hpText.emplace(game->mainFont);
        hpText->setCharacterSize(static_cast<int>(16 * GLOBAL_FONT_SCALE));
        hpText->setFillColor(sf::Color::White);

        // --- Tekst Fali (Wave) ---
        waveText.emplace(game->mainFont);
        waveText->setCharacterSize(static_cast<int>(28 * GLOBAL_FONT_SCALE));
        waveText->setFillColor(sf::Color(237, 224, 221));
        waveText->setOutlineThickness(7.5f);
        waveText->setOutlineColor(sf::Color::Black);
        waveText->setStyle(sf::Text::Bold);

        // --- Biomasa (Fasolka) ---
        if (rm.hasTexture("magic_bean_base")) {
            biomassSprite.emplace(*rm.getTexture("magic_bean_base"));
            auto size = biomassSprite->getTexture().getSize();
            biomassSprite->setScale({ 2.0f, 2.0f });
            biomassSprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
            biomassSprite->setPosition({ 35.0f, 65.0f });
        }

        biomassText.emplace(game->mainFont);
        biomassText->setCharacterSize(static_cast<int>(18 * GLOBAL_FONT_SCALE));
        biomassText->setFillColor(sf::Color(10, 230, 255));
        biomassText->setOutlineThickness(1.5f);
        biomassText->setOutlineColor(sf::Color::Black);
        biomassText->setPosition({ 55.f, 65.f });

        // --- Celownik (Crosshair) ---
        if (rm.hasTexture("ui_crosshair")) {
            crosshairSprite.emplace(*rm.getTexture("ui_crosshair"));
            auto size = crosshairSprite->getTexture().getSize();
            crosshairSprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
            game->getWindow().setMouseCursorVisible(false);
        }

        // --- Przycisk Ustawien (Pauza) ---
        setupButton("ui_settings", settingsBtnSprite, { viewSize.x - 50.0f, 50.0f }, { 60.0f, 60.0f });
    }

    // --- GAME LOOP ---

    void PlayingState::handleEvent(const sf::Event& event)
    {
        if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
            if (scroll->delta > 0) cameraView.zoom(0.9f);
            else if (scroll->delta < 0) cameraView.zoom(1.1f);
        }

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                game->getStateMachine().pushState(StateType::Pause);
                game->getWindow().setMouseCursorVisible(true);
            }

            // Player input
            auto* player = world->getPlayer();
            if (player != nullptr) {
                if (keyPressed->code == sf::Keyboard::Key::LShift) {
                    sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()), cameraView);
                    if (auto* ab = player->getComponent<game::components::AbilityComponent>()) ab->useSkill(mouseWorldPos);
                }
                if (keyPressed->code == sf::Keyboard::Key::Space) {
                    if (auto* stats = player->getComponent<game::components::StatsComponent>()) {
                        if (stats->isUltReady()) {
                            if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                                auto* player_transform = player->getComponent<game::components::TransformComponent>();
                                if (!player_transform) return;

                                ab->useUltimate(player_transform->position);
                                stats->resetUlt();
                                shakeIntensity = 15.0f;
                            }
                        }
                    }
                }
            }
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                sf::Vector2f uiPos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()), game->getWindow().getDefaultView());

                // Klikniecie pauzy
                if (settingsBtnSprite && settingsBtnSprite->getGlobalBounds().contains(uiPos)) {
                    game->playUIClick();
                    game->getStateMachine().pushState(StateType::Pause);
                    game->getWindow().setMouseCursorVisible(true);
                }
            }
        }
    }

    void PlayingState::update(float dt)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H)) {
            shakeIntensity = 20.0f;
            sf::sleep(sf::milliseconds(45));
        }

        // 1. Logika awiata gry
        world->update(dt);

        // 2. Input ataku gracza
        auto* player = world->getPlayer();
        if (player && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()), cameraView);
            if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                ab->useWeapon(mouseWorldPos);
            }
        }

        // 3. Aktualizacje kamery i UI
        updateHUD();
        updateCamera(dt);

        // Czysty, dziedziczony hover dla przycisku Settings
        sf::Vector2f uiHoverPos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()), game->getWindow().getDefaultView());
        updateHover(settingsBtnSprite, { 60.f, 60.f }, uiHoverPos);

        // 4. Przejscie do sklepu
        if (world->requiresShop()) {
            world->resolveShopBreak();
            game->getWindow().setMouseCursorVisible(true);
            game->getStateMachine().pushState(StateType::Shop);
        }
    }

    void PlayingState::updateHUD()
    {
        auto* player = world->getPlayer();
        if (player) {
            if (auto* stats = player->getComponent<game::components::StatsComponent>()) {

                // --- DEATH DETECT ---
                if (stats->getHealth() <= 0.0f)
                {
                    std::cout << "[PlayingState] Player is dead!\n";

                    if (stats->getLastDamageSourceKey().empty()) {
                        game->lastSessionResults.killerNameKey = "unknown_organism";
                    }
                    else {
                        game->lastSessionResults.killerNameKey = stats->getLastDamageSourceKey();
                    }
                    game->lastSessionResults.wavesSurvived = world->getEvolutionManager()->getCurrentWave() - 1;
                    game->lastSessionResults.biomassCollected = game->profile.biomassJuice;

                    // Przejscie na ekran smierci
                    game->getStateMachine().pushState(StateType::Death);
                    game->getWindow().setMouseCursorVisible(true);
                    return;
                }

                // --- HP BAR UPDATE ---
                float currentHp = std::max(0.0f, stats->getHealth());
                hpBarFill.setSize({ 200.f * (currentHp / stats->getMaxHealth()), 20.f });
                hpText->setString(std::to_string(static_cast<int>(currentHp)) + " / " + std::to_string(static_cast<int>(stats->getMaxHealth())));

                sf::FloatRect bounds = hpText->getLocalBounds();
                hpText->setOrigin({ std::round(bounds.size.x / 2.0f), std::round(bounds.position.y + bounds.size.y / 2.0f) });
                hpText->setPosition({ std::round(hpBarBg.getPosition().x + 100.0f), std::round(hpBarBg.getPosition().y + 10.0f) });
            }
        }

        // --- WAVE UPDATE ---
        waveText->setString(LocUTF8("ui_wave") + " " + std::to_string(world->getEvolutionManager()->getCurrentWave()));
        sf::FloatRect waveBounds = waveText->getLocalBounds();
        waveText->setOrigin({ std::round(waveBounds.size.x / 2.0f), std::round(waveBounds.position.y + waveBounds.size.y / 2.0f) });
        waveText->setPosition({ std::round(game->getWindow().getView().getSize().x / 2.0f), 60.f });

        // --- BIOMASS UPDATE ---
        if (biomassText) {
            biomassText->setString(std::to_string(game->profile.biomassJuice));
        }
    }

    void PlayingState::updateCamera(float dt)
    {
        auto* player = world->getPlayer();  if (!player) return;
        auto* player_transform = player->getComponent<game::components::TransformComponent>();  if (!player_transform) return;

        sf::Vector2f viewSize = cameraView.getSize();
        float hW = viewSize.x / 2.0f, hH = viewSize.y / 2.0f;
        sf::Vector2f center = player_transform->position;

        center.x = std::clamp(center.x, hW, mapLimits.x - hW);
        center.y = std::clamp(center.y, hH, mapLimits.y - hH);
        cameraView.setCenter(center);

        if (shakeIntensity > 0.0f) {
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_real_distribution<float> offset(-shakeIntensity, shakeIntensity);
            cameraView.move({ offset(gen), offset(gen) });

            shakeIntensity -= dt * 50.0f;
            if (shakeIntensity < 0.0f) shakeIntensity = 0.0f;
        }

        game->getWindow().setView(cameraView);
    }

    void PlayingState::renderHUD(sf::RenderWindow& window)
    {
        window.draw(hpBarBg);
        window.draw(hpBarFill);

        if (hpText) window.draw(*hpText);
        if (waveText) window.draw(*waveText);
        if (biomassSprite) window.draw(*biomassSprite);
        if (biomassText) window.draw(*biomassText);
        if (settingsBtnSprite) window.draw(*settingsBtnSprite);
    }

    void PlayingState::render(sf::RenderWindow& window)
    {
        // 1. World Render (with Camera offset)
        window.setView(cameraView);
        world->render(window, mapSprite);

        // 2. HUD Render (Static on screen)
        window.setView(window.getDefaultView());

        // Custom crosshair
        if (game->getStateMachine().getCurrentStateType() == states::StateType::Playing && crosshairSprite) {
            crosshairSprite->setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getDefaultView()));
            window.draw(*crosshairSprite);
        }

        renderHUD(window);
    }
}