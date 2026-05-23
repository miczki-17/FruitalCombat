// --- PlayingState.cpp --- 


#include "PlayingState.h"
#include "../factories/FruitFactory.h"
#include "../core/ArenaContext.h"
#include "../core/ResourceManager.h"
#include "../components/AbilityComponent.h"
#include "../components/SpriteComponent.h"
#include <algorithm>
#include <iostream>
#include <random> 

namespace game::states
{
    PlayingState::PlayingState(game::Game* game)
        : State(game)
    {
        std::cout << "[PLAYING STATE] Building arena elements...\n";

        std::string mapKey = game->selectedMapKey;
        const auto& mapData = game->mapsConfig[mapKey];
        std::string mapMaskPath = mapData.value("maskPath", "");

        // ?ADOWANIE Z RESOURCE MANAGERA! ZERO DYSKU W TRAKCIE GRY!
        if (game::core::ResourceManager::get().hasTexture(mapKey + "_map"))
        {
            mapSprite.emplace(game::core::ResourceManager::get().getTexture(mapKey + "_map"));
            mapSprite->setScale({ mapScale, mapScale });

            sf::Vector2u rawSize = game::core::ResourceManager::get().getTexture(mapKey + "_map").getSize();
            mapLimits = sf::Vector2f(rawSize.x * mapScale, rawSize.y * mapScale);
        }

        if (!collisionMask.loadFromFile(mapMaskPath))
        {
            std::cerr << "[ERROR] Could not load collision mask from: " << mapMaskPath << '\n';
        }

        
        // --- HUD INIT ---
        // bar bg
        hpBarBg.setSize({ 200.f, 20.f });
        hpBarBg.setPosition({ 20.f, 20.f });
        hpBarBg.setFillColor(sf::Color(40, 40, 40, 200));
        hpBarBg.setOutlineThickness(2.f);
        hpBarBg.setOutlineColor(sf::Color::Black);

        // bar infill
        hpBarFill.setSize({ 200.f, 20.f });
        hpBarFill.setPosition({ 20.f, 20.f });
        hpBarFill.setFillColor(sf::Color(220, 30, 30));

        // bar text
        hpText.emplace(game->mainFont);
        hpText->setCharacterSize(16);
        hpText->setFillColor(sf::Color::Black);


        // waves ctr
        waveText.emplace(game->mainFont);
        waveText->setCharacterSize(28);
        waveText->setFillColor(sf::Color(237, 224, 221));
        waveText->setOutlineThickness(7.5f);
        waveText->setOutlineColor(sf::Color::Black);
        waveText->setStyle(sf::Text::Bold);


        // --- BIOMASS HUD ---
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



        if (game->menuUiBuffer.contains("crosshair"))
        {
            if (crosshairTex.loadFromImage(game->menuUiBuffer["crosshair"]))
            {
                crosshairSprite.emplace(crosshairTex);
                sf::Vector2u size = crosshairTex.getSize();
                crosshairSprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
                game->getWindow().setMouseCursorVisible(false);
            }
        }

        // Cache effects vector pipeline buffers
        game->arenaContext.splashTextures.clear();
        game->arenaContext.dynamicZones.clear();
        game->arenaContext.acidSplashes.clear();

        for (int i = 1; i <= 3; i++)
        {
            auto tex = std::make_shared<sf::Texture>();
            if (tex->loadFromFile("assets/textures/entities/characters/citrus_maximus/orange_acid_splash_" + std::to_string(i) + ".png"))
            {
                tex->setSmooth(true);
                game->arenaContext.splashTextures.push_back(tex);
            }
        }

        auto setupButton = [&](const std::string& key, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
            {
                if (game->menuUiBuffer.contains(key))
                {
                    if (tex.loadFromImage(game->menuUiBuffer[key]))
                    {
                        spr = sf::Sprite(tex);
                        sf::Vector2f originalSize(tex.getSize());
                        float scaleX = targetSize.x / originalSize.x;
                        float scaleY = targetSize.y / originalSize.y;
                        (*spr).setScale({ scaleX, scaleY });
                        (*spr).setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });
                        (*spr).setPosition(pos);
                    }
                }
            };

        sf::Vector2f viewSize = game->getWindow().getView().getSize();
        float margin = 20.0f;
        setupButton("settings", settingsBtnTex, settingsBtnSprite, { 0.0f, 0.0f }, { 60.0f, 60.0f });

        if (settingsBtnSprite.has_value()) {
            sf::FloatRect bounds = (*settingsBtnSprite).getGlobalBounds();
            (*settingsBtnSprite).setPosition({ viewSize.x - margin - (bounds.size.x / 2.0f), margin + (bounds.size.y / 2.0f) });
        }


        // walk particles
        int dr = mapData.value("dustR", 150);
        int dg = mapData.value("dustG", 150);
        int db = mapData.value("dustB", 150);
        int da = mapData.value("dustA", 150);

        game->arenaContext.currentMapDustColor = sf::Color(
            static_cast<std::uint8_t>(dr),
            static_cast<std::uint8_t>(dg),
            static_cast<std::uint8_t>(db),
            static_cast<std::uint8_t>(da)
        );


        // Connect global processing pipelines
        game->arenaContext.bullets = &bullets;

        game::factories::FruitFactory factory(game->arenaContext, game->fruitsConfig, game, collisionMask, mapScale);
        player = factory.createFruit(game->selectedFruitType);

        if (player != nullptr)
        {
            player->position = { mapLimits.x / 2.0f, mapLimits.y / 2.0f };
        }

        mutantFactory = 
            std::make_unique<game::factories::MutantFactory>(
                game->arenaContext,
                game, 
                collisionMask, 
                mapScale);

        evolutionManager = 
            std::make_unique<game::systems::EvolutionManager>(
                *mutantFactory,
                enemies,
                player.get(),
                game->enemiesConfig,
                collisionMask,
                mapScale);

        // Odpalamy pierwsz? fal?
        evolutionManager->startFirstWave();

        cameraView = game->getWindow().getDefaultView();
        cameraView.zoom(1.4f);
    }

    StateType PlayingState::getType() const { return StateType::Playing; }

    void PlayingState::handleEvent(const sf::Event& event)
    {
        if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (scroll->delta > 0)      cameraView.zoom(0.9f);
            else if (scroll->delta < 0) cameraView.zoom(1.1f);
        }

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                game->getStateMachine().pushState(StateType::Pause);
            }
            if (keyPressed->code == sf::Keyboard::Key::LShift && player != nullptr)
            {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, cameraView);

                if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                    ab->useSkill(mouseWorldPos);
                }
            }
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f uiPos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());
                if (settingsBtnSprite.has_value() && settingsBtnSprite->getGlobalBounds().contains(uiPos))
                {
                    game->playUIClick();
                    game->getStateMachine().pushState(StateType::Pause);
                }
            }
        }
    }

    void PlayingState::update(float dt)
    {
        static float shakeIntensity = 0.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H))
        {
            shakeIntensity = 20.0f;
            sf::sleep(sf::milliseconds(45));
        }

        if (player != nullptr)
        {
            player->update(dt);


            // --- JUICE COLLECTION LOGIC (Przyci?ganie magnesem) ---
            for (int i = static_cast<int>(game->arenaContext.juiceDrops.size()) - 1; i >= 0; --i)
            {
                sf::Vector2f diff = player->position - game->arenaContext.juiceDrops[i].position;
                float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                if (dist < 180.0f) // Zasi?g magnesu
                {
                    sf::Vector2f pullDir = diff / dist;
                    game->arenaContext.juiceDrops[i].position += pullDir * 350.0f * dt;
                    game->arenaContext.juiceDrops[i].shape.setPosition(game->arenaContext.juiceDrops[i].position);
                }

                if (dist < 30.0f) // Zasi?g zebrania (wch?oni?cia)
                {
                    game->playerJuice += static_cast<int>(game->arenaContext.juiceDrops[i].value);
                    game->arenaContext.juiceDrops.erase(game->arenaContext.juiceDrops.begin() + i);
                }
            }

            // --- FLOATING TEXT UPDATE (Aktualizacja czasu ?ycia cyferek) ---
            for (int i = static_cast<int>(game->arenaContext.damageTexts.size()) - 1; i >= 0; --i) {
                game->arenaContext.damageTexts[i].update(dt);
                if (game->arenaContext.damageTexts[i].isDead()) {
                    game->arenaContext.damageTexts.erase(game->arenaContext.damageTexts.begin() + i);
                }
            }


            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, cameraView);

                // Uzycie przez ECS
                if (auto* ab = player->getComponent<game::components::AbilityComponent>()) {
                    ab->useWeapon(mouseWorldPos);
                }
            }

            // Centralized projectile lifecycle and splash management tracking loop
            for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; --i)
            {
                sf::Vector2f explodePos = bullets[i].getPosition();
                bullets[i].update(dt, collisionMask, mapScale);

                if (bullets[i].consumeSplash())
                {
                    if (!game->arenaContext.splashTextures.empty())
                    {
                        int randomTexIdx = rand() % game->arenaContext.splashTextures.size();

                        game->arenaContext.acidSplashes.emplace_back(
                            explodePos,
                            game->arenaContext.splashTextures[randomTexIdx]
                        );
                    }
                }

                if (!bullets[i].getIsActive())
                {
                    if (bullets[i].getRadius() > 10.0f)
                    {
                        if (!game->arenaContext.splashTextures.empty())
                        {
                            AoEZone puddle;
                            puddle.radius = 55.0f;

                            int randomTexIdx = rand() % game->arenaContext.splashTextures.size();

                            puddle.shape.setRadius(puddle.radius);
                            puddle.shape.setOrigin({ puddle.radius, puddle.radius });
                            puddle.shape.setPosition(explodePos);

                            puddle.shape.setFillColor(sf::Color::Transparent);

                            puddle.shape.setOutlineColor(sf::Color::Transparent);
                            puddle.shape.setOutlineThickness(0.0f);

                            puddle.lifetime = 4.0f;
                            puddle.maxLifetime = 4.0f;

                            game->arenaContext.dynamicZones.push_back(puddle);
                        }

                        shakeIntensity = 8.0f;
                    }
                    bullets.erase(bullets.begin() + i);
                }
            }

            // Update hazard zones safely mapped outside local static scope
            for (int i = static_cast<int>(game->arenaContext.dynamicZones.size()) - 1; i >= 0; --i)
            {
                auto& puddle = game->arenaContext.dynamicZones[i];
                puddle.lifetime -= dt;

                if (puddle.lifetime <= 0.0f)
                {
                    game->arenaContext.dynamicZones.erase(game->arenaContext.dynamicZones.begin() + i);
                    continue;
                }

                float alphaFactor = puddle.lifetime / puddle.maxLifetime;
                uint8_t alpha = static_cast<uint8_t>(alphaFactor * 255.0f);

                puddle.shape.setFillColor(sf::Color::Transparent);
                puddle.shape.setOutlineColor(sf::Color::Transparent);
            }



            // --- HUD update ---
            // player hp bar
            if (auto* stats = player->getComponent<game::components::StatsComponent>())
            {
                float currentHp = std::max(0.0f, stats->hp);
                float hpRatio = currentHp / stats->maxHp;

                hpBarFill.setSize({ 200.f * hpRatio, 20.f });

                hpText->setString(std::to_string(static_cast<int>(currentHp)) + " / " + std::to_string(static_cast<int>(stats->maxHp)));

                sf::FloatRect hpBounds = hpText->getLocalBounds();

                hpText->setOrigin({
                    hpBounds.position.x + (hpBounds.size.x / 2.0f),
                    hpBounds.position.y + (hpBounds.size.y / 2.0f)
                    });

                float yOffset = 5.0f;

                hpText->setPosition({
                    hpBarBg.getPosition().x + (hpBarBg.getSize().x / 2.0f),
                    hpBarBg.getPosition().y + (hpBarBg.getSize().y / 2.0f) + yOffset
                    });
            }

            const int currentWave = evolutionManager->getCurrentWave();

            waveText->setString("Wave " + std::to_string(currentWave));
            sf::FloatRect waveBounds = waveText->getLocalBounds();
            waveText->setOrigin({ waveBounds.size.x / 2.0f, waveBounds.size.y / 2.0f });

            // top px
            float windowCenterX = game->getWindow().getView().getSize().x / 2.0f;
            waveText->setPosition({ windowCenterX, 60.f });

            if (biomassText.has_value()) {
                biomassText->setString(std::to_string(static_cast<int>(game->playerJuice)));
            }



            // --- COMBAT / COLLISION RESOLUTION ---
            float defaultBulletDamage = 25.0f;

            for (auto& bullet : bullets)
            {
                if (!bullet.getIsActive()) continue;

                // Check collision against all enemies
                for (auto& enemy : enemies)
                {
                    if (enemy->isDead) continue;

                    // Simple circular distance collision
                    sf::Vector2f diff = bullet.getPosition() - enemy->position;
                    float distSq = diff.x * diff.x + diff.y * diff.y;

                    // We estimate enemy hit radius based on their ColliderComponent or a default 25px
                    float enemyHitRadius = 25.0f;
                    if (auto* collider = enemy->getComponent<game::components::ColliderComponent>()) {
                        enemyHitRadius = 30.0f;
                    }

                    float combinedRadius = bullet.getRadius() + enemyHitRadius;

                    if (distSq < combinedRadius * combinedRadius)
                    {
                        // Hit detected!
                        bullet.destroy(); // Consumes the bullet

                        // Apply damage
                        if (auto* stats = enemy->getComponent<game::components::StatsComponent>()) {
                            stats->takeDamage(defaultBulletDamage);

                            // 1. HIT FLASH TRIGGER! (Zmienia wroga na chwil? na bia?o)
                            if (auto* sprite = enemy->getComponent<game::components::SpriteComponent>()) {
                                sprite->triggerHitFlash();
                            }

							// 2. SPAWN DAMAGE TEXT
                            game->arenaContext.damageTexts.emplace_back(
                                game->mainFont,
                                "-" + std::to_string(static_cast<int>(defaultBulletDamage)),
                                enemy->position,
                                sf::Color::Red
                            );
                        }
                        break; // Bullet can only hit one enemy
                    }
                }
            }



            // --- AKTUALIZACJA ISTNIEJ¥CYCH CZ¥STECZEK KURZU ---
            for (int i = static_cast<int>(game->arenaContext.walkParticles.size()) - 1; i >= 0; --i) {
                game->arenaContext.walkParticles[i].lifetime -= dt;

                if (game->arenaContext.walkParticles[i].lifetime <= 0.0f) {
                    game->arenaContext.walkParticles.erase(game->arenaContext.walkParticles.begin() + i);
                }
                else {
                    game->arenaContext.walkParticles[i].position += game->arenaContext.walkParticles[i].velocity * dt;
                    game->arenaContext.walkParticles[i].velocity *= 0.95f;
                }
            }

            // --- GENEROWANIE KURZU GDY GRACZ BIEGNIE ---
            static sf::Vector2f lastPlayerPos = player->position;
            sf::Vector2f moveDelta = player->position - lastPlayerPos;

            // Jeœli gracz przesun¹³ siê o wiêcej ni¿ u³amek piksela, to znaczy, ¿e siê rusza!
            bool isActuallyMoving = (moveDelta.x * moveDelta.x + moveDelta.y * moveDelta.y) > 0.1f;
            lastPlayerPos = player->position; // Zapisujemy pozycjê na kolejn¹ klatkê

            if (isActuallyMoving) {
                playerDustSpawnTimer -= dt;
                if (playerDustSpawnTimer <= 0.0f) {
                    playerDustSpawnTimer = 0.06f;

                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> offsetDist(-12.0f, 12.0f);
                    std::uniform_real_distribution<float> velXDist(-25.0f, 25.0f);
                    std::uniform_real_distribution<float> velYDist(-15.0f, 5.0f);
                    std::uniform_real_distribution<float> sizeDist(4.0f, 8.0f);
                    std::uniform_real_distribution<float> lifeDist(0.3f, 0.6f);

                    sf::Vector2f feetPos = player->position + sf::Vector2f(0.0f, 18.0f);

                    game->arenaContext.walkParticles.emplace_back(
                        feetPos + sf::Vector2f(offsetDist(gen), offsetDist(gen)),
                        sf::Vector2f(velXDist(gen), velYDist(gen)),
                        lifeDist(gen),
                        sizeDist(gen),
                        game->arenaContext.currentMapDustColor
                    );
                }
            }
            else {
                playerDustSpawnTimer = 0.0f;
            }



            // --- ENEMY UPDATE & GRAVEYARD CLEANUP ---
            for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; --i)
            {
                enemies[i]->update(dt);

                if (enemies[i]->isDead)
                {
                    // Harvest DNA
                    if (auto* dnaComp = enemies[i]->getComponent<game::components::DNAComponent>()) {
                        evolutionManager->onEnemyDeath(dnaComp->dna);

                        // Rzut na drop (losuje liczbê od 0.00 do 1.00)
                        float randomRoll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                        // Sprawdzamy, czy wylosowana liczba mieœci siê w szansie na drop z JSON-a
                        if (randomRoll <= dnaComp->dna.dropChance)
                        {
                            // U¿ywamy zmutowanej bazy z DNA pomno¿onej przez wielkoœæ mutanta!
                            float xpValue = dnaComp->dna.baseJuice * dnaComp->dna.sizeScale;
                            game->arenaContext.juiceDrops.emplace_back(enemies[i]->position, xpValue);
                        }
                    }
                    // Remove from arena
                    enemies.erase(enemies.begin() + i);
                }
            }

            // Evolution Manager checks if wave is complete
            evolutionManager->update(dt);

            // --- PRZEJŒCIE DO SKLEPU CO 3 FALE ---
            if (evolutionManager->requiresShop())
            {
                evolutionManager->resolveShopBreak();
                game->getWindow().setMouseCursorVisible(true);
                game->getStateMachine().pushState(StateType::Shop);
            }



            // Camera calculations and clamping restrictions
            sf::Vector2f viewSize = cameraView.getSize();
            float halfWidth = viewSize.x / 2.0f;
            float halfHeight = viewSize.y / 2.0f;
            sf::Vector2f targetCenter = player->position;

            float minX = halfWidth, maxX = mapLimits.x - halfWidth;
            float minY = halfHeight, maxY = mapLimits.y - halfHeight;

            targetCenter.x = (maxX < minX) ? mapLimits.x / 2.0f : std::clamp(targetCenter.x, minX, maxX);
            targetCenter.y = (maxY < minY) ? mapLimits.y / 2.0f : std::clamp(targetCenter.y, minY, maxY);
            cameraView.setCenter(targetCenter);

            if (shakeIntensity > 0.0f)
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> offset(-shakeIntensity, shakeIntensity);
                cameraView.move({ offset(gen), offset(gen) });

                shakeIntensity -= dt * 50.0f;
                if (shakeIntensity < 0.0f) shakeIntensity = 0.0f;
            }
            game->getWindow().setView(cameraView);

            // Menu overlay interface button configurations
            sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
            sf::Vector2f uiHoverPos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());

            auto updateHover = [&](std::optional<sf::Sprite>& btn, sf::Vector2f targetSize) {
                if (!btn) return;
                sf::Vector2f texSize(btn->getTexture().getSize());
                float baseScaleX = targetSize.x / texSize.x;
                float baseScaleY = targetSize.y / texSize.y;

                if (btn->getGlobalBounds().contains(uiHoverPos)) {
                    btn->setColor(sf::Color(255, 255, 255));
                    btn->setScale({ baseScaleX * 1.1f, baseScaleY * 1.1f });
                }
                else {
                    btn->setColor(sf::Color(210, 210, 210));
                    btn->setScale({ baseScaleX, baseScaleY });
                }
                };
            updateHover(settingsBtnSprite, { 60.0f, 60.0f });

            for (auto& splash : game->arenaContext.acidSplashes) {
                splash.update(dt);
            }

            game->arenaContext.acidSplashes.erase(
                std::remove_if(game->arenaContext.acidSplashes.begin(), game->arenaContext.acidSplashes.end(),
                    [](const auto& s) { return !s.isActive(); }),
                game->arenaContext.acidSplashes.end());
        }
    }


    void PlayingState::renderHUD(sf::RenderWindow& window) 
    {
        // HUD render
        window.draw(hpBarBg);
        window.draw(hpBarFill);
        if (hpText.has_value()) window.draw(*hpText);
        if (waveText.has_value()) window.draw(*waveText);
    }


    void PlayingState::render(sf::RenderWindow& window)
    {
        window.setView(cameraView);
        if (mapSprite.has_value()) window.draw(*mapSprite);

        // Draw circles underneath other processing entities
        for (const auto& zone : game->arenaContext.dynamicZones) {
            window.draw(zone.shape);
        }

        for (auto& bullet : bullets) {
            bullet.render(window);
        }


        // --- WALKING PARTICLES ---
        for (const auto& p : game->arenaContext.walkParticles) {
            sf::RectangleShape dustShape({ p.size * 2.0f, p.size * 2.0f });

            dustShape.setOrigin({ p.size, p.size });
            dustShape.setPosition(p.position);

            float lifeRatio = p.lifetime / p.maxLifetime;
            sf::Color c = p.color;
            c.a = static_cast<std::uint8_t>(c.a * lifeRatio);

            dustShape.setFillColor(c);
            window.draw(dustShape);
        }


        if (player != nullptr) {
            player->render(window);
        }

        for (auto& splash : game->arenaContext.acidSplashes) {
            splash.render(window);
        }

        for (auto& enemy : enemies) {
            enemy->render(window);
        }

        for (auto& drop : game->arenaContext.juiceDrops) {
            drop.render(window);
        }

        for (auto& text : game->arenaContext.damageTexts) {
            text.render(window);
        }

        window.setView(window.getDefaultView());
        if (settingsBtnSprite.has_value()) window.draw(*settingsBtnSprite);
        renderHUD(window);

        if (game->getStateMachine().getCurrentStateType() == states::StateType::Playing)
        {
            if (crosshairSprite.has_value())
            {
                sf::View oldView = window.getView();
                window.setView(window.getDefaultView());
                sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
                sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel, window.getDefaultView());
                crosshairSprite->setPosition(mouseWorld);
                window.draw(*crosshairSprite);
                window.setView(oldView);
            }
        }


        renderHUD(window);

        window.draw(biomassIcon);
        if (biomassText.has_value()) window.draw(*biomassText);
    }
}