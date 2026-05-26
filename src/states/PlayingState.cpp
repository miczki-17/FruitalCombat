// states/PlayingState.cpp

#include "PlayingState.h"
#include "../factories/FruitFactory.h"
#include "../core/ArenaContext.h"
#include "../core/ResourceManager.h"
#include "../components/AbilityComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/MovementComponent.h"
#include <algorithm>
#include <iostream>
#include <random>

namespace game::states
{
    // INICJALIZACJA I SEKCJA STARTOWA

    PlayingState::PlayingState(game::Game* game) : State(game)
    {
        std::cout << "[PlayingState] Inicjalizacja areny i systemow...\n";

        // --- 1. Mapa i fizyka terenu ---
        std::string mapKey = game->selectedMapKey;
        const auto& mapData = game->mapsConfig[mapKey];
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

        // --- 2. Czyszczenie wspó?dzielonego ArenaContext ---
        game->arenaContext.entities.clear();
        game->arenaContext.splashTextures.clear();
        game->arenaContext.zones.clear();
        game->arenaContext.acidSplashes.clear();
        game->arenaContext.bullets.clear();
        game->arenaContext.floatingTexts.clear();
        game->arenaContext.juiceDrops.clear();
        game->arenaContext.walkParticles.clear();

        // ?adowanie wizualnych rozbryzgów kwasu
        for (int i = 1; i <= 3; i++) {
            auto tex = std::make_shared<sf::Texture>();
            if (tex->loadFromFile("assets/textures/entities/characters/citrus_maximus/orange_acid_splash_" + std::to_string(i) + ".png")) {
                tex->setSmooth(true);
                game->arenaContext.splashTextures.push_back(tex);
            }
        }

        game->arenaContext.mapDustColor = sf::Color(
            static_cast<std::uint8_t>(mapData.value("dustR", 150)),
            static_cast<std::uint8_t>(mapData.value("dustG", 150)),
            static_cast<std::uint8_t>(mapData.value("dustB", 150)),
            static_cast<std::uint8_t>(mapData.value("dustA", 150))
        );

        // --- 3. Inicjalizacja HUD ---
        initHUD();

        // --- 4. Fabryki i narodziny Gracza ---
        game::factories::FruitFactory factory(game->arenaContext, game->fruitsConfig, collisionMask, mapScale, enemies);
        player = factory.createFruit(game->selectedFruitType);

        if (player != nullptr) {
            player->position = sf::Vector2f(mapLimits.x / 2.0f, mapLimits.y / 2.0f);
            lastPlayerPos = player->position;

            game->arenaContext.playerStats = player->getComponent<game::components::StatsComponent>();

            if (auto* moveComp = player->getComponent<game::components::MovementComponent>()) {
                moveComp->setGamePointer(game);
            }
        }

        // --- 5. Fabryka Mutantów i Evolution Manager ---
        mutantFactory = std::make_unique<game::factories::MutantFactory>(game->arenaContext, game, collisionMask, mapScale);
        evolutionManager = std::make_unique<game::systems::EvolutionManager>(
            *mutantFactory, enemies, player.get(), game->enemiesConfig, collisionMask, mapScale
        );

        evolutionManager->startFirstWave();

        // --- 6. POWO?ANIE NOWYCH SYSTEMÓW DO ?YCIA ---
        collisionSystem = std::make_unique<game::systems::CollisionSystem>(game->arenaContext, enemies);
        combatSystem = std::make_unique<game::systems::CombatSystem>(game, game->arenaContext, enemies);
        particleSystem = std::make_unique<game::systems::ParticleSystem>(game->arenaContext);
        renderSystem = std::make_unique<game::systems::RenderSystem>(game->arenaContext);

        // --- 7. Ustawienie Kamery SFML ---
        cameraView = game->getWindow().getDefaultView();
        cameraView.zoom(1.4f);
    }

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

        if (game->menuUiBuffer.contains("crosshair") && crosshairTex.loadFromImage(game->menuUiBuffer["crosshair"])) {
            crosshairSprite.emplace(crosshairTex);
            crosshairSprite->setOrigin({ crosshairTex.getSize().x / 2.0f, crosshairTex.getSize().y / 2.0f });
            game->getWindow().setMouseCursorVisible(false);
        }

        if (game->menuUiBuffer.contains("settings") && settingsBtnTex.loadFromImage(game->menuUiBuffer["settings"])) {
            settingsBtnSprite = sf::Sprite(settingsBtnTex);
            settingsBtnSprite->setScale({ 60.0f / settingsBtnTex.getSize().x, 60.0f / settingsBtnTex.getSize().y });
            settingsBtnSprite->setOrigin({ settingsBtnTex.getSize().x / 2.0f, settingsBtnTex.getSize().y / 2.0f });

            sf::Vector2f viewSize = game->getWindow().getView().getSize();
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
            if (player != nullptr) {
                if (keyPressed->code == sf::Keyboard::Key::LShift) {
                    sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
                    sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, cameraView);
                    if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                        ab->useSkill(mouseWorldPos);
                    }
                }
                if (keyPressed->code == sf::Keyboard::Key::Space) {
                    if (auto* stats = player->getComponent<game::components::StatsComponent>()) {
                        if (stats->isUltReady()) {
                            if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                                ab->useUltimate(player->position);
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

    // G?ÓWNA P?TLA AKTUALIZACJI (ZUNIFIKOWANE PIPELINE SYSTEMOWE)

    void PlayingState::update(float dt)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H)) {
            shakeIntensity = 20.0f;
            sf::sleep(sf::milliseconds(45));
        }

        if (player == nullptr) return;

        // --- 1. Aktualizacja Komponentów Stats (Buffy, DoTy, Cooldowny) ---
        if (auto* stats = player->getComponent<game::components::StatsComponent>()) {
            stats->update(dt);
        }
        for (auto& enemy : enemies) {
            if (auto* stats = enemy->getComponent<game::components::StatsComponent>()) {
                stats->update(dt);
            }
        }

        // --- 2. Aktualizacja bazowych Encji ---
        player->update(dt);
        for (auto& enemy : enemies) {
            enemy->update(dt);
        }

        // --- 3. Obs?uga ci?g?ego ataku podstawowego broni? ---
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
            sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, cameraView);
            if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                ab->useWeapon(mouseWorldPos);
            }
        }

        // --- 4. WYWO?ANIA NOWYCH MODU?ÓW SYSTEMOWYCH ---
        collisionSystem->updateJuiceCollection(player.get(), dt);

        // ZMIANA: Przekazujemy parametry do zaktualizowanego systemu kolizji pocisków!
        collisionSystem->updateBulletIntersections(dt, collisionMask, mapScale);

        combatSystem->processJuiceCollection(player.get());
        combatSystem->processBulletDamage();

        particleSystem->updateEffects(dt);
        particleSystem->updateParticles(player.get(), dt, lastPlayerPos, playerDustSpawnTimer);

        combatSystem->processEnemyDeaths(*evolutionManager);

        // --- 5. Logika pomocnicza i HUD ---
        updateHUD();
        updateCamera(dt);
        handleUIHover();

        // --- 6. Aktualizacja fal ewolucyjnych potworów ---
        evolutionManager->update(dt);
        if (evolutionManager->requiresShop()) {
            evolutionManager->resolveShopBreak();
            game->getWindow().setMouseCursorVisible(true);
            game->getStateMachine().pushState(StateType::Shop);
        }
    }

    // LOGIKA APARATU I INTERFEJSU

    void PlayingState::updateHUD()
    {
        if (auto* stats = player->getComponent<game::components::StatsComponent>()) {
            float currentHp = std::max(0.0f, stats->getHealth());
            hpBarFill.setSize({ 200.f * (currentHp / stats->getMaxHealth()), 20.f });
            hpText->setString(std::to_string(static_cast<int>(currentHp)) + " / " + std::to_string(static_cast<int>(stats->getMaxHealth())));

            sf::FloatRect bounds = hpText->getLocalBounds();
            hpText->setOrigin({ bounds.position.x + (bounds.size.x / 2.0f), bounds.position.y + (bounds.size.y / 2.0f) });
            hpText->setPosition({ hpBarBg.getPosition().x + 100.0f, hpBarBg.getPosition().y + 10.0f });
        }

        waveText->setString("Wave " + std::to_string(evolutionManager->getCurrentWave()));
        sf::FloatRect waveBounds = waveText->getLocalBounds();
        waveText->setOrigin({ waveBounds.size.x / 2.0f, waveBounds.size.y / 2.0f });
        waveText->setPosition({ game->getWindow().getView().getSize().x / 2.0f, 60.f });

        if (biomassText.has_value()) {
            biomassText->setString(std::to_string(game->profile.biomassJuice));
        }
    }

    void PlayingState::updateCamera(float dt)
    {
        sf::Vector2f viewSize = cameraView.getSize();
        float hW = viewSize.x / 2.0f, hH = viewSize.y / 2.0f;
        sf::Vector2f center = player->position;
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
        sf::Vector2f uiHoverPos = game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(game->getWindow()), game->getWindow().getDefaultView());
        if (settingsBtnSprite) {
            if (settingsBtnSprite->getGlobalBounds().contains(uiHoverPos)) {
                settingsBtnSprite->setColor(sf::Color::White);
                settingsBtnSprite->setScale({ 1.1f * (60.0f / settingsBtnTex.getSize().x), 1.1f * (60.0f / settingsBtnTex.getSize().y) });
            }
            else {
                settingsBtnSprite->setColor(sf::Color(210, 210, 210));
                settingsBtnSprite->setScale({ 60.0f / settingsBtnTex.getSize().x, 60.0f / settingsBtnTex.getSize().y });
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

        renderSystem->renderWorld(window, mapSprite, player.get(), enemies);

        window.setView(window.getDefaultView());

        if (game->getStateMachine().getCurrentStateType() == states::StateType::Playing && crosshairSprite.has_value()) {
            crosshairSprite->setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getDefaultView()));
            window.draw(*crosshairSprite);
        }

        renderHUD(window);
    }

    StateType PlayingState::getType() const
    {
        return StateType::Playing;
	}
}