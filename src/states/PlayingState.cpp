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
    // =========================================================================
    // INITIALIZATION & SETUP
    // =========================================================================
    PlayingState::PlayingState(game::Game* game) : State(game)
    {
        std::cout << "[PlayingState] Initializing arena elements...\n";

        // --- 1. Map & Physics Initialization ---
        std::string mapKey = game->selectedMapKey;
        const auto& mapData = game->mapsConfig[mapKey];
        std::string mapMaskPath = mapData.value("maskPath", "");

        if (game::core::ResourceManager::get().hasTexture(mapKey + "_map")) {
            mapSprite.emplace(*game::core::ResourceManager::get().getTexture(mapKey + "_map"));
            mapSprite->setScale(sf::Vector2f(mapScale, mapScale));
            sf::Vector2u rawSize = game::core::ResourceManager::get().getTexture(mapKey + "_map")->getSize();
            mapLimits = sf::Vector2f(rawSize.x * mapScale, rawSize.y * mapScale);
        }

        if (!collisionMask.loadFromFile(mapMaskPath)) {
            std::cerr << "[ERROR] Could not load collision mask from: " << mapMaskPath << '\n';
        }

        // --- 2. Arena Context Setup ---
        game->arenaContext.splashTextures.clear();
        game->arenaContext.zones.clear();
        game->arenaContext.acidSplashes.clear();
        game->arenaContext.bullets.clear();
        game->arenaContext.floatingTexts.clear();

        // Preload visual splash effects
        for (int i = 1; i <= 3; i++) {
            auto tex = std::make_shared<sf::Texture>();
            if (tex->loadFromFile("assets/textures/entities/characters/citrus_maximus/orange_acid_splash_" + std::to_string(i) + ".png")) {
                tex->setSmooth(true);
                game->arenaContext.splashTextures.push_back(tex);
            }
        }

        // Set environmental dust color based on the map config
        game->arenaContext.mapDustColor = sf::Color(
            static_cast<std::uint8_t>(mapData.value("dustR", 150)),
            static_cast<std::uint8_t>(mapData.value("dustG", 150)),
            static_cast<std::uint8_t>(mapData.value("dustB", 150)),
            static_cast<std::uint8_t>(mapData.value("dustA", 150))
        );

        // --- 3. UI Initialization ---
        initHUD();

        // --- 4. Entity Factories & Player Spawning ---
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

        // --- 5. Enemy Management & Evolution ---
        mutantFactory = std::make_unique<game::factories::MutantFactory>(game->arenaContext, game, collisionMask, mapScale);
        evolutionManager = std::make_unique<game::systems::EvolutionManager>(
            *mutantFactory, enemies, player.get(), game->enemiesConfig, collisionMask, mapScale
            );

        evolutionManager->startFirstWave();

        // --- 6. Camera Setup ---
        cameraView = game->getWindow().getDefaultView();
        cameraView.zoom(1.4f);
    }

    void PlayingState::initHUD()
    {
        hpBarBg.setSize(sf::Vector2f(200.f, 20.f));
        hpBarBg.setFillColor(sf::Color(40, 40, 40, 200));
        hpBarBg.setOutlineThickness(2.f);
        hpBarBg.setOutlineColor(sf::Color::Black);

        hpBarFill.setSize(sf::Vector2f(200.f, 20.f));
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
        biomassIcon.setOrigin(sf::Vector2f(8.0f, 8.0f));
        biomassIcon.setFillColor(sf::Color(10, 230, 255, 220));
        biomassIcon.setOutlineThickness(1.5f);
        biomassIcon.setOutlineColor(sf::Color::White);
        biomassIcon.setPosition(sf::Vector2f(30.f, 65.f));

        biomassText.emplace(game->mainFont);
        biomassText->setCharacterSize(18);
        biomassText->setFillColor(sf::Color(10, 230, 255));
        biomassText->setOutlineThickness(1.5f);
        biomassText->setOutlineColor(sf::Color::Black);
        biomassText->setPosition(sf::Vector2f(45.f, 53.f));

        if (game->menuUiBuffer.contains("crosshair") && crosshairTex.loadFromImage(game->menuUiBuffer["crosshair"])) {
            crosshairSprite.emplace(crosshairTex);
            crosshairSprite->setOrigin(sf::Vector2f(crosshairTex.getSize().x / 2.0f, crosshairTex.getSize().y / 2.0f));
            game->getWindow().setMouseCursorVisible(false);
        }

        if (game->menuUiBuffer.contains("settings") && settingsBtnTex.loadFromImage(game->menuUiBuffer["settings"])) {
            settingsBtnSprite = sf::Sprite(settingsBtnTex);
            settingsBtnSprite->setScale(sf::Vector2f(60.0f / settingsBtnTex.getSize().x, 60.0f / settingsBtnTex.getSize().y));
            settingsBtnSprite->setOrigin(sf::Vector2f(settingsBtnTex.getSize().x / 2.0f, settingsBtnTex.getSize().y / 2.0f));

            sf::Vector2f viewSize = game->getWindow().getView().getSize();
            settingsBtnSprite->setPosition(sf::Vector2f(viewSize.x - 50.0f, 50.0f));
        }
    }

    StateType PlayingState::getType() const { return StateType::Playing; }

    // =========================================================================
    // INPUT HANDLING
    // =========================================================================
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

    // =========================================================================
    // MAIN GAME LOOP (UPDATE PIPELINE)
    // =========================================================================
    void PlayingState::update(float dt)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H)) {
            shakeIntensity = 20.0f;
            sf::sleep(sf::milliseconds(45));
        }

        if (player == nullptr) return;

        // --- Update Smart Stats (Buffs, DoTs, Cooldowns) ---
        if (auto* stats = player->getComponent<game::components::StatsComponent>()) {
            stats->update(dt);
        }
        for (auto& enemy : enemies) {
            if (auto* stats = enemy->getComponent<game::components::StatsComponent>()) {
                stats->update(dt);
            }
        }

        player->update(dt);

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
            sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, cameraView);
            if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                ab->useWeapon(mouseWorldPos);
            }
        }

        updateJuiceDrops(dt);
        updateCombat(dt);
        updateEffects(dt);
        updateParticles(dt);
        updateEnemies(dt);
        updateHUD();
        updateCamera(dt);
        handleUIHover();

        evolutionManager->update(dt);
        if (evolutionManager->requiresShop()) {
            evolutionManager->resolveShopBreak();
            game->getWindow().setMouseCursorVisible(true);
            game->getStateMachine().pushState(StateType::Shop);
        }
    }

    // =========================================================================
    // SUBSYSTEM IMPLEMENTATIONS
    // =========================================================================
    void PlayingState::updateJuiceDrops(float dt)
    {
        auto& drops = game->arenaContext.juiceDrops;

        for (int i = static_cast<int>(drops.size()) - 1; i >= 0; --i) {
            sf::Vector2f diff = player->position - drops[i].position;
            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

            // Magnet effect
            if (dist < 180.0f) {
                drops[i].position += (diff / dist) * 350.0f * dt;
                drops[i].shape.setPosition(drops[i].position);
            }

            // Collection
            if (dist < 30.0f) {
                float juiceValue = drops[i].value;
                game->playerJuice += static_cast<int>(juiceValue);

                if (auto* stats = player->getComponent<game::components::StatsComponent>()) {
                    stats->addUltCharge(juiceValue * 0.5f);
                }

                drops.erase(drops.begin() + i);
            }
        }
    }

    void PlayingState::updateCombat(float dt)
    {
        float defaultBulletDamage = 25.0f;
        auto& bullets = game->arenaContext.bullets; // Referencja dla wygody

        for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; --i) {
            sf::Vector2f explodePos = bullets[i].getPosition();
            bullets[i].update(dt, collisionMask, mapScale);

            // Mid-air Collision Detection
            if (bullets[i].getIsActive()) {
                for (auto& enemy : enemies) {
                    if (enemy->isDead) continue;

                    float combinedRadius = bullets[i].getRadius() + 30.0f;
                    sf::Vector2f diff = explodePos - enemy->position;

                    if ((diff.x * diff.x + diff.y * diff.y) < (combinedRadius * combinedRadius)) {
                        bullets[i].destroy();
                        break;
                    }
                }
            }

            // Explosion & Splash Logic
            if (!bullets[i].getIsActive()) {
                float explosionRadius = 70.0f;

                for (auto& enemy : enemies) {
                    if (enemy->isDead) continue;

                    sf::Vector2f diff = explodePos - enemy->position;
                    if ((diff.x * diff.x + diff.y * diff.y) < (explosionRadius * explosionRadius)) {
                        if (auto* stats = enemy->getComponent<game::components::StatsComponent>()) {
                            stats->takeDamage(defaultBulletDamage);

                            if (auto* sprite = enemy->getComponent<game::components::SpriteComponent>()) {
                                sprite->triggerHitFlash();
                            }

                            // U¿ywamy nowej nazwy: floatingTexts
                            game->arenaContext.floatingTexts.emplace_back(
                                game->mainFont,
                                "-" + std::to_string(static_cast<int>(defaultBulletDamage)),
                                enemy->position,
                                sf::Color::Red
                            );
                        }
                    }
                }

                if (!game->arenaContext.splashTextures.empty()) {
                    int randomTexIdx = rand() % game->arenaContext.splashTextures.size();
                    game->arenaContext.acidSplashes.emplace_back(explodePos, game->arenaContext.splashTextures[randomTexIdx]);
                }

                if (bullets[i].getStatusEffect() == game::components::StatusEffect::Poison) {
                    AoEZone puddle;
                    puddle.radius = 65.0f;
                    puddle.dps = 15.0f;

                    puddle.shape.setRadius(puddle.radius);
                    puddle.shape.setOrigin(sf::Vector2f(puddle.radius, puddle.radius));
                    puddle.shape.setPosition(explodePos);
                    puddle.shape.setFillColor(sf::Color::Transparent);
                    puddle.shape.setOutlineThickness(0.0f);

                    puddle.lifetime = 4.0f;
                    puddle.maxLifetime = 4.0f;

                    // U¿ywamy nowej nazwy: zones
                    game->arenaContext.zones.push_back(puddle);
                    shakeIntensity = 8.0f;
                }

                bullets.erase(bullets.begin() + i);
            }
        }
    }

    void PlayingState::updateEffects(float dt)
    {
        // U¿ywamy nowej nazwy: floatingTexts
        auto& texts = game->arenaContext.floatingTexts;
        for (int i = static_cast<int>(texts.size()) - 1; i >= 0; --i) {
            texts[i].update(dt);
            if (texts[i].isDead()) {
                texts.erase(texts.begin() + i);
            }
        }

        // U¿ywamy nowej nazwy: zones
        auto& zones = game->arenaContext.zones;
        for (int i = static_cast<int>(zones.size()) - 1; i >= 0; --i) {
            auto& puddle = zones[i];
            puddle.lifetime -= dt;

            for (auto& enemy : enemies) {
                if (enemy->isDead) continue;
                sf::Vector2f diff = enemy->position - puddle.shape.getPosition();

                if ((diff.x * diff.x + diff.y * diff.y) < (puddle.radius * puddle.radius)) {
                    if (auto* stats = enemy->getComponent<game::components::StatsComponent>()) {

                        if (puddle.dps > 0.0f) stats->takeDamage(puddle.dps * dt);

                        if (puddle.appliesPoison) {
                            stats->addEffect(game::components::StatusType::Poison, 1.0f, puddle.poisonDps);
                        }
                        if (puddle.appliesSlow) {
                            stats->addEffect(game::components::StatusType::Slow, 0.5f, puddle.slowMultiplier);
                        }
                    }
                }
            }

            if (puddle.lifetime <= 0.0f) {
                zones.erase(zones.begin() + i);
                continue;
            }
        }

        auto& splashes = game->arenaContext.acidSplashes;
        for (auto& splash : splashes) splash.update(dt);
        splashes.erase(std::remove_if(splashes.begin(), splashes.end(),
            [](const auto& s) { return !s.isActive(); }), splashes.end());
    }

    void PlayingState::updateParticles(float dt)
    {
        auto& particles = game->arenaContext.walkParticles;

        for (int i = static_cast<int>(particles.size()) - 1; i >= 0; --i) {
            particles[i].lifetime -= dt;
            if (particles[i].lifetime <= 0.0f) {
                particles.erase(particles.begin() + i);
            }
            else {
                particles[i].position += particles[i].velocity * dt;
                particles[i].velocity *= 0.95f;
            }
        }

        sf::Vector2f moveDelta = player->position - lastPlayerPos;
        bool isMoving = (moveDelta.x * moveDelta.x + moveDelta.y * moveDelta.y) > 0.1f;
        lastPlayerPos = player->position;

        if (isMoving) {
            playerDustSpawnTimer -= dt;
            if (playerDustSpawnTimer <= 0.0f) {
                playerDustSpawnTimer = 0.06f;
                std::random_device rd; std::mt19937 gen(rd());
                std::uniform_real_distribution<float> offsetDist(-12.0f, 12.0f);
                std::uniform_real_distribution<float> velDist(-25.0f, 25.0f);
                std::uniform_real_distribution<float> sizeDist(4.0f, 8.0f);

                particles.emplace_back(
                    player->position + sf::Vector2f(offsetDist(gen), offsetDist(gen) + 18.0f),
                    sf::Vector2f(velDist(gen), velDist(gen) - 10.0f),
                    0.4f, sizeDist(gen), game->arenaContext.mapDustColor // U¿yto mapDustColor
                );
            }
        }
        else {
            playerDustSpawnTimer = 0.0f;
        }
    }

    void PlayingState::updateEnemies(float dt)
    {
        for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; --i) {
            enemies[i]->update(dt);

            if (auto* stats = enemies[i]->getComponent<game::components::StatsComponent>()) {
                if (stats->getHealth() <= 0.0f) {
                    enemies[i]->isDead = true;
                }
            }

            if (enemies[i]->isDead) {
                if (auto* dnaComp = enemies[i]->getComponent<game::components::DNAComponent>()) {
                    const auto& dnaData = dnaComp->getDNA();

                    evolutionManager->onEnemyDeath(dnaData);

                    float randomRoll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    if (randomRoll <= dnaData.dropChance) {
                        game->arenaContext.juiceDrops.emplace_back(enemies[i]->position, dnaData.baseJuice * dnaData.sizeScale);
                    }
                }
                enemies.erase(enemies.begin() + i);
            }
        }
    }

    void PlayingState::updateHUD()
    {
        if (auto* stats = player->getComponent<game::components::StatsComponent>()) {
            float currentHp = std::max(0.0f, stats->getHealth());
            hpBarFill.setSize(sf::Vector2f(200.f * (currentHp / stats->getMaxHealth()), 20.f));
            hpText->setString(std::to_string(static_cast<int>(currentHp)) + " / " + std::to_string(static_cast<int>(stats->getMaxHealth())));

            sf::FloatRect bounds = hpText->getLocalBounds();
            hpText->setOrigin(sf::Vector2f(bounds.position.x + (bounds.size.x / 2.0f), bounds.position.y + (bounds.size.y / 2.0f)));
            hpText->setPosition(sf::Vector2f(hpBarBg.getPosition().x + 100.0f, hpBarBg.getPosition().y + 10.0f));
        }

        waveText->setString("Wave " + std::to_string(evolutionManager->getCurrentWave()));
        sf::FloatRect waveBounds = waveText->getLocalBounds();
        waveText->setOrigin(sf::Vector2f(waveBounds.size.x / 2.0f, waveBounds.size.y / 2.0f));
        waveText->setPosition(sf::Vector2f(game->getWindow().getView().getSize().x / 2.0f, 60.f));

        if (biomassText.has_value()) {
            biomassText->setString(std::to_string(static_cast<int>(game->playerJuice)));
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
            cameraView.move(sf::Vector2f(offset(gen), offset(gen)));
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
                settingsBtnSprite->setScale(sf::Vector2f(1.1f * (60.0f / settingsBtnTex.getSize().x), 1.1f * (60.0f / settingsBtnTex.getSize().y)));
            }
            else {
                settingsBtnSprite->setColor(sf::Color(210, 210, 210));
                settingsBtnSprite->setScale(sf::Vector2f(60.0f / settingsBtnTex.getSize().x, 60.0f / settingsBtnTex.getSize().y));
            }
        }
    }

    // =========================================================================
    // RENDERING
    // =========================================================================
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
        if (mapSprite.has_value()) window.draw(*mapSprite);

        // Pociski korzystaj¹ teraz z wektora z ArenaContext
        for (auto& bullet : game->arenaContext.bullets) bullet.render(window);

        for (const auto& p : game->arenaContext.walkParticles) {
            sf::RectangleShape dustShape(sf::Vector2f(p.size * 2.0f, p.size * 2.0f));
            dustShape.setOrigin(sf::Vector2f(p.size, p.size));
            dustShape.setPosition(p.position);
            sf::Color c = p.color;
            c.a = static_cast<std::uint8_t>(c.a * (p.lifetime / p.maxLifetime));
            dustShape.setFillColor(c);
            window.draw(dustShape);
        }

        if (player != nullptr) player->render(window);
        for (auto& splash : game->arenaContext.acidSplashes) splash.render(window);
        for (auto& enemy : enemies) enemy->render(window);
        for (auto& drop : game->arenaContext.juiceDrops) drop.render(window);

        // Teksty trafieñ korzystaj¹ z nowej nazwy
        for (auto& text : game->arenaContext.floatingTexts) text.render(window);

        window.setView(window.getDefaultView());

        if (game->getStateMachine().getCurrentStateType() == states::StateType::Playing && crosshairSprite.has_value()) {
            crosshairSprite->setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getDefaultView()));
            window.draw(*crosshairSprite);
        }

        renderHUD(window);
    }
}