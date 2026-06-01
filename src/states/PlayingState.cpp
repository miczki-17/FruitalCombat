// --- PlayingState.cpp ---

#include "PlayingState.h"
#include "../core/ResourceManager.h"
#include "../core/AudioManager.h"
#include "../components/AbilityComponent.h"
#include "../components/StatsComponent.h"
#include "../components/TransformComponent.h"
#include "../systems/EvolutionManager.h"
#include <algorithm>
#include <iostream>
#include <random>

namespace game::states
{
    PlayingState::PlayingState(game::Game* game) : State(game)
    {
        std::cout << "[PlayingState] World initialization...\n";

        // clear RAM
        game::core::AudioManager::get().stopMusic();
        game::core::AudioManager::get().stopAllSounds();
        game::core::ResourceManager::get().unloadGroup(game::core::AssetGroup::Menu);
        game::core::ResourceManager::get().unloadGroup(game::core::AssetGroup::Intro);


        auto& rm = game::core::ResourceManager::get();

        // loading entities textures
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

            // effects
            if (fruitData.contains("splashKey") && fruitData.contains("splashProjectileTexturePath"))
            {
                loadedSplashKey_ = fruitData.value("splashKey", "");
                std::string basePath = fruitData.value("splashProjectileTexturePath", "");

                
                for (int i = 1; i <= 3; i++) {
                    std::string finalKey = loadedSplashKey_ + "_" + std::to_string(i);
                    std::string finalPath = basePath + std::to_string(i) + ".png";
                    rm.loadTexture(finalKey, finalPath, game::core::AssetGroup::Playing);
                }
                std::cout << "[PlayingState] Loaded splash textures under key: " << loadedSplashKey_ << "\n";
            }

            // textures & animations - character

            if (fruitData.contains("walkTexturePath")) {
                std::string path = fruitData.value("walkTexturePath", "");
                if (!path.empty()) rm.loadTexture(fruitKey + "_walk", path, game::core::AssetGroup::Playing);
            }
        }

        // textures & animations - enemies
        for (auto& [enemyKey, enemyData] : game->enemiesConfig.items()) {
            if (enemyData.contains("idleTexturePath")) {
                rm.loadTexture(enemyKey + "_idle", enemyData.value("idleTexturePath", ""), game::core::AssetGroup::Playing);
            }
            if (enemyData.contains("walkTexturePath")) {
                rm.loadTexture(enemyKey + "_walk", enemyData.value("walkTexturePath", ""), game::core::AssetGroup::Playing);
            }
        }


        // loading maps
        std::string mapKey = game->selectedMapKey;
        const auto& mapData = game->mapsConfig[mapKey];

        // arena backgrounds
        if (mapData.contains("texturePath")) {
            rm.loadTexture(mapKey + "_map", mapData.value("texturePath", ""), game::core::AssetGroup::Playing);
        }

        std::string mapMaskPath = mapData.value("maskPath", "");

        if (game::core::ResourceManager::get().hasTexture(mapKey + "_map")) {
            mapSprite.emplace(*game::core::ResourceManager::get().getTexture(mapKey + "_map"));
            mapSprite->setScale({ mapScale, mapScale });
            sf::Vector2u rawSize = game::core::ResourceManager::get().getTexture(mapKey + "_map")->getSize();
            mapLimits = sf::Vector2f(rawSize.x * mapScale, rawSize.y * mapScale);
        }

        if (!collisionMask.loadFromFile(mapMaskPath)) {
            std::cerr << "[ERROR] Could not load collision mask from: " << mapMaskPath << '\n';
        }

        // --- Loading resources dependent on the map ---
        if (mapKey == "CrisperDrawer")
        {
            rm.loadTexture("hazard_icicle", "assets/textures/hazards/icicle.png", game::core::AssetGroup::Playing);
            rm.loadTexture("hazard_icicle_shard", "assets/textures/hazards/icicle_shard.png", game::core::AssetGroup::Playing);
            rm.loadSound("icicle_shatter", "assets/sounds/icicle_shatter.mp3", game::core::AssetGroup::Playing);
        }
        else if (mapKey == "ChoppingBlock")
        {
            rm.loadTexture("hazard_knife", "assets/textures/hazards/knife.png", game::core::AssetGroup::Playing);
            rm.loadSound("knife_hit", "assets/sounds/knife_hit.mp3", game::core::AssetGroup::Playing);
        }
        else if (mapKey == "WildOrchard")
        {
            rm.loadTexture("hazard_spore", "assets/textures/hazards/spore.png", game::core::AssetGroup::Playing);
            rm.loadTexture("hazard_spore_splash", "assets/textures/hazards/spore_splash_1.png", game::core::AssetGroup::Playing);
            rm.loadSound("spore_splat", "assets/sounds/spore_splat.mp3", game::core::AssetGroup::Playing);
        }



        // HUD resources
        for (uint8_t i = 0; i <= 4; i++)
        {
            std::string path = "assets/textures/entities/drops/biomass_juice_" + std::to_string(i) + ".png";
            rm.loadTexture("juice_drop_" + std::to_string(i), path, game::core::AssetGroup::Playing);
        }
        

        game->arenaContext.mapDustColor = sf::Color(
            static_cast<std::uint8_t>(mapData.value("dustR", 150)),
            static_cast<std::uint8_t>(mapData.value("dustG", 150)),
            static_cast<std::uint8_t>(mapData.value("dustB", 150)),
            static_cast<std::uint8_t>(mapData.value("dustA", 150))
        );

        initHUD();

        // Create the game world with the loaded map and collision data
        sf::Vector2f startPos(mapLimits.x / 2.0f, mapLimits.y / 2.0f);
        world = std::make_unique<game::core::GameWorld>(game, collisionMask, mapScale, startPos);

        cameraView = game->getWindow().getDefaultView();
        cameraView.zoom(1.4f);
    }

	// CLEAR RAM RESOURCES DEPENDING ON THE MAP
    PlayingState::~PlayingState()
    {
        game::core::AudioManager::get().stopMusic();
        game::core::AudioManager::get().stopAllSounds();
        game::core::ResourceManager::get().unloadGroup(game::core::AssetGroup::Playing);
        game::core::ResourceManager::get().unloadGroup(game::core::AssetGroup::Global);         // to change
    }



    StateType PlayingState::getType() const { return StateType::Playing; }



    void PlayingState::initHUD()
    {
        hpBarBg.setSize({ 200.f, 20.f });
        hpBarBg.setFillColor(sf::Color(40, 40, 40, 200));
        hpBarBg.setOutlineThickness(2.f);
        hpBarBg.setOutlineColor(sf::Color::Black);

        hpBarFill.setSize({ 200.f, 20.f });
        hpBarFill.setFillColor(sf::Color(220, 30, 30));

        hpText.emplace(game->mainFont);
        hpText->setCharacterSize(16);
        hpText->setFillColor(sf::Color::White);

        waveText.emplace(game->mainFont);
        waveText->setCharacterSize(28);
        waveText->setFillColor(sf::Color(237, 224, 221));
        waveText->setOutlineThickness(7.5f);
        waveText->setOutlineColor(sf::Color::Black);
        waveText->setStyle(sf::Text::Bold);

        biomassIcon.setRadius(8.0f);
        biomassIcon.setOrigin({ 8.0f, 8.0f });
        biomassIcon.setFillColor(sf::Color(10, 230, 255, 220));
        biomassIcon.setOutlineThickness(1.5f);
        biomassIcon.setOutlineColor(sf::Color::White);
        biomassIcon.setPosition({ 30.f, 65.f });

        biomassText.emplace(game->mainFont);
        biomassText->setCharacterSize(18);
        biomassText->setFillColor(sf::Color(10, 230, 255));
        biomassText->setOutlineThickness(1.5f);
        biomassText->setOutlineColor(sf::Color::Black);
        biomassText->setPosition({ 45.f, 53.f });

		auto& rm = game::core::ResourceManager::get();

        if (rm.hasTexture("ui_crosshair"))
        {
            crosshairSprite.emplace( *rm.getTexture("ui_crosshair") );
            auto size = crosshairSprite->getTexture().getSize();
            crosshairSprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
            game->getWindow().setMouseCursorVisible(false);
        }

        if (rm.hasTexture("ui_settings"))
        {
            settingsBtnSprite.emplace(*rm.getTexture("ui_settings"));
            const auto size = settingsBtnSprite->getTexture().getSize();

            settingsBtnSprite->setScale({ 60.0f / size.x, 60.0f / size.y });
            settingsBtnSprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
            sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();
            settingsBtnSprite->setPosition({ viewSize.x - 50.0f, 50.0f });
        }
    }

    void PlayingState::handleEvent(const sf::Event& event)
    {
        if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
            if (scroll->delta > 0) cameraView.zoom(0.9f);
            else if (scroll->delta < 0) cameraView.zoom(1.1f);
        }

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                game->getStateMachine().pushState(StateType::Pause);
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
                                auto* player_transform = player->getComponent<game::components::TransformComponent>(); if (!player_transform) return;
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
                if (settingsBtnSprite.has_value() && settingsBtnSprite->getGlobalBounds().contains(uiPos)) {
                    game->playUIClick();
                    game->getStateMachine().pushState(StateType::Pause);
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

        // 1. Main game logic update
        world->update(dt);

        // 2. continuous player input (weapon aiming)
        auto* player = world->getPlayer();
        if (player && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()), cameraView);
            if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                ab->useWeapon(mouseWorldPos);
            }
        }

        // 3. HUD, camera and UI updates
        updateHUD();
        updateCamera(dt);
        handleUIHover();

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
                float currentHp = std::max(0.0f, stats->getHealth());
                hpBarFill.setSize({ 200.f * (currentHp / stats->getMaxHealth()), 20.f });
                hpText->setString(std::to_string(static_cast<int>(currentHp)) + " / " + std::to_string(static_cast<int>(stats->getMaxHealth())));

                sf::FloatRect bounds = hpText->getLocalBounds();
                hpText->setOrigin({ bounds.position.x + (bounds.size.x / 2.0f), bounds.position.y + (bounds.size.y / 2.0f) });
                hpText->setPosition({ hpBarBg.getPosition().x + 100.0f, hpBarBg.getPosition().y + 10.0f });
            }
        }

        waveText->setString("Wave " + std::to_string(world->getEvolutionManager()->getCurrentWave()));
        sf::FloatRect waveBounds = waveText->getLocalBounds();
        waveText->setOrigin({ waveBounds.size.x / 2.0f, waveBounds.size.y / 2.0f });
        waveText->setPosition({ game->getWindow().getView().getSize().x / 2.0f, 60.f });

        // player biomass juice update
        if (biomassText.has_value()) {
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

    void PlayingState::handleUIHover()
    {
        sf::Vector2f uiHoverPos =
            game->getWindow().mapPixelToCoords(
                sf::Mouse::getPosition(game->getWindow()),
                game->getWindow().getDefaultView());

        if (settingsBtnSprite)
        {
            auto size =
                settingsBtnSprite->getTexture().getSize();

            float baseScaleX = 60.0f / size.x;
            float baseScaleY = 60.0f / size.y;

            if (settingsBtnSprite->getGlobalBounds().contains(uiHoverPos))
            {
                settingsBtnSprite->setColor(sf::Color::White);

                settingsBtnSprite->setScale({
                    baseScaleX * 1.1f,
                    baseScaleY * 1.1f
                    });
            }
            else
            {
                settingsBtnSprite->setColor(
                    sf::Color(210, 210, 210));

                settingsBtnSprite->setScale({
                    baseScaleX,
                    baseScaleY
                    });
            }
        }
    }

    void PlayingState::renderHUD(sf::RenderWindow& window)
    {
        window.draw(hpBarBg);
        window.draw(hpBarFill);
        if (hpText.has_value()) window.draw(*hpText);
        if (waveText.has_value()) window.draw(*waveText);
        window.draw(biomassIcon);
        if (biomassText.has_value()) window.draw(*biomassText);
        if (settingsBtnSprite.has_value()) window.draw(*settingsBtnSprite);
    }

    void PlayingState::render(sf::RenderWindow& window)
    {
        window.setView(cameraView);

        // Game world rendering
        world->render(window, mapSprite);

        window.setView(window.getDefaultView());

        if (game->getStateMachine().getCurrentStateType() == states::StateType::Playing && crosshairSprite.has_value()) {
            crosshairSprite->setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getDefaultView()));
            window.draw(*crosshairSprite);
        }

        renderHUD(window);
    }
}