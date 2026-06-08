// --- CombatSystem.cpp ---

#include "CombatSystem.h"
#include "../core/Game.h"
#include "../core/ArenaContext.h"
#include "../core/ResourceManager.h"
#include "../core/AudioManager.h"
#include "../entities/Entity.h"
#include "../components/StatsComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/DNAComponent.h"
#include "../components/ProjectileComponent.h"
#include "../components/TransformComponent.h"
#include "../components/TextComponent.h"
#include "../components/LifespanComponent.h"
#include "../components/PopAnimationComponent.h"
#include "../components/JuiceComponent.h"
#include "../components/AoEComponent.h"
#include "EvolutionManager.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <random>

namespace game::systems
{
    CombatSystem::CombatSystem(game::Game* game, game::ArenaContext& context, std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef)
        : game_(game), context_(context), enemies_(enemiesRef)
    {
    }

    void CombatSystem::processJuiceCollection(game::entities::Entity* player)
    {
        if (!player || player->isDead()) return;

        auto& entities = context_.entities;
        for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i)
        {
            if (auto* juice = entities[i]->getComponent<game::components::JuiceComponent>())
            {
                if (juice->isCollected)
                {
                    // Sprawdzamy, czy to MONETA do sklepu
                    if (juice->isCoin)
                    {
                        // std::round gwarantuje, ¿e 0.9999f stanie siê 1, a nie 0
                        game_->profile.addCoins(static_cast<int>(std::round(juice->value)));
                    }
                    else
                    {
                        game_->profile.addJuice(static_cast<int>(std::round(juice->value)));

                        if (auto* stats = player->getComponent<game::components::StatsComponent>()) {
                            stats->addUltCharge(juice->value * 0.5f);
                            stats->restoreMana(juice->value * 3.0f);
                        }
                    }

                    // Niezale¿nie od tego czy to moneta czy sok, po zebraniu niszczymy encjê
                    entities[i]->destroy();
                }
            }
        }
    }

    void CombatSystem::processBulletDamage(game::entities::Entity* player)
    {
        auto& entities = context_.entities;

        for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i)
        {
            auto* proj = entities[i]->getComponent<game::components::ProjectileComponent>();
            if (!proj) continue;

            if (proj->getIsActive()) continue;

            sf::Vector2f explodePos = proj->getPosition();
            float explosionRadius = 70.0f;
            float actualDamage = proj->getDamage();

            // 1. AREA DAMAGE: WROGOWIE
            for (auto& enemy : enemies_)
            {
                if (enemy->isDead()) continue;

                auto* enemy_transform = enemy->getComponent<game::components::TransformComponent>();
                if (!enemy_transform) continue;

                sf::Vector2f diff = explodePos - enemy_transform->position;

                if (diff.x * diff.x + diff.y * diff.y < (explosionRadius * explosionRadius))
                {
                    if (auto* stats = enemy->getComponent<game::components::StatsComponent>())
                    {
                        stats->takeDamage(actualDamage);

                        // --- NAK£ADANIE EFEKTÓW ---
                        if (proj->getStatusEffect() == game::components::StatusEffect::Slow) {
                            // Spowalnia 50% przez 2s
                            stats->addEffect(game::components::StatusType::Slow, 2.0f, 0.5f);
                        }

                        if (auto* sprite = enemy->getComponent<game::components::SpriteComponent>()) {
                            sprite->triggerHitFlash();
                        }

                        auto textEntity = std::make_unique<game::entities::Entity>();
                        if (auto* transform = textEntity->getComponent<game::components::TransformComponent>()) {
                            transform->position = enemy_transform->position;
                            transform->velocity = sf::Vector2f(static_cast<float>((rand() % 100) - 50), -140.0f);
                        }

                        textEntity->addComponent(std::make_unique<game::components::TextComponent>(
                            game_->mainFont, "-" + std::to_string(static_cast<int>(actualDamage)), 22, sf::Color::Red));
                        textEntity->addComponent(std::make_unique<game::components::LifespanComponent>(0.6f, true));
                        context_.spawnEntity(std::move(textEntity));
                    }
                }
            }

            // 2. AREA DAMAGE: GRACZ
            if (!proj->getIsFriendly() && player && !player->isDead())
            {
                auto* player_transform = player->getComponent<game::components::TransformComponent>();
                if (player_transform)
                {
                    sf::Vector2f diffToPlayer = explodePos - player_transform->position;
                    if (diffToPlayer.x * diffToPlayer.x + diffToPlayer.y * diffToPlayer.y < (explosionRadius * explosionRadius))
                    {
                        if (auto* stats = player->getComponent<game::components::StatsComponent>())
                        {
                            stats->takeDamage(actualDamage);
                            //stats->setLastDamageSourceKey();

                            // --- NAK£ADANIE EFEKTÓW ---
                            //if (proj->getStatusEffect() == game::components::StatusEffect::Slow) {
                            //    // Spowalnia 50% przez 2s
                            //    stats->addEffect(game::components::StatusType::Slow, 2.0f, 0.5f);
                            //}

                            if (auto* sprite = player->getComponent<game::components::SpriteComponent>()) {
                                sprite->triggerHitFlash();
                            }

                            auto textEntity = std::make_unique<game::entities::Entity>();
                            if (auto* transform = textEntity->getComponent<game::components::TransformComponent>()) {
                                transform->position = player_transform->position;
                                transform->velocity = sf::Vector2f(static_cast<float>((rand() % 100) - 50), -140.0f);
                            }

                            textEntity->addComponent(std::make_unique<game::components::TextComponent>(
                                game_->mainFont, "-" + std::to_string(static_cast<int>(actualDamage)), 22, sf::Color::Red));
                            textEntity->addComponent(std::make_unique<game::components::LifespanComponent>(0.6f, true));
                            context_.spawnEntity(std::move(textEntity));
                        }
                    }
                }
            }

            // 3. WIZUALNY EFEKT ROZPRYSKU (SPLASH - ECS!)
            if (proj->getStatusEffect() != game::components::StatusEffect::IceShatter &&
                proj->getStatusEffect() != game::components::StatusEffect::SporePoison)
            {
                std::string baseKey = proj->getSplashKeyBase();
                if (!baseKey.empty())
                {
                    std::string splashKey = baseKey + "_" + std::to_string((std::rand() % 3) + 1);
                    auto& rm = game::core::ResourceManager::get();

                    if (rm.hasTexture(splashKey)) {
                        auto splashEntity = std::make_unique<game::entities::Entity>();
                        if (auto* transform = splashEntity->getComponent<game::components::TransformComponent>()) {
                            transform->position = explodePos;
                            // Generowanie losowego k¹ta w stopniach
                            transform->rotation = static_cast<float>(rand() % 360);

                            float baseScale = 0.55f + static_cast<float>(rand() % 40) / 100.0f;
                            transform->scale = { baseScale * 1.35f, baseScale * 0.7f };

                            splashEntity->addComponent(std::make_unique<game::components::PopAnimationComponent>(
                                transform->scale, sf::Vector2f(baseScale, baseScale), 10.0f));
                        }

                        auto spriteComp = std::make_unique<game::components::SpriteComponent>();
                        spriteComp->setTexture(rm.getTextureShared(splashKey));
                        splashEntity->addComponent(std::move(spriteComp));
                        splashEntity->addComponent(std::make_unique<game::components::LifespanComponent>(1.5f, true));

                        context_.spawnEntity(std::move(splashEntity));
                    }
                }
            }


            // 4. AOE DLA TRUCIZNY 
            if (proj->getStatusEffect() == game::components::StatusEffect::Poison)
            {
                float r = 80.0f;
                float dps = 5.0f;

                auto aoeEntity = std::make_unique<game::entities::Entity>();
                if (auto* transform = aoeEntity->getComponent<game::components::TransformComponent>()) {
                    transform->position = explodePos;
                }

                auto poisonAoE = std::make_unique<game::components::AoEComponent>(
                    120.0f, sf::Color(100, 200, 255, 80), 40.0f, false, 0.0f, true, 0.4f, proj->getIsFriendly());
                poisonAoE->isVisible = false;
                aoeEntity->addComponent(std::move(poisonAoE));

                // Dodajemy czas zycia z wygaszaniem 
                aoeEntity->addComponent(std::make_unique<game::components::LifespanComponent>(5.0f, true));

                context_.spawnEntity(std::move(aoeEntity));
            }


            // 5. Spore AoE
            if (proj->getStatusEffect() == game::components::StatusEffect::SporePoison)
            {
                float r = 120.0f;
                float dps = 25.0f;

                auto sporeEntity = std::make_unique<game::entities::Entity>();
                if (auto* transform = sporeEntity->getComponent<game::components::TransformComponent>()) {
                    transform->position = explodePos;
                }

                auto sporeAoE = std::make_unique<game::components::AoEComponent>(
                    120.0f, sf::Color(100, 200, 255, 80), 40.0f, false, 0.0f, true, 0.4f);
                sporeAoE->isVisible = false;
                sporeEntity->addComponent(std::move(sporeAoE));

                std::string baseKey = proj->getSplashKeyBase();
                auto& rm = game::core::ResourceManager::get();
                if (!baseKey.empty() && rm.hasTexture(baseKey)) {
                    auto splashEntity = std::make_unique<game::entities::Entity>();
                    if (auto* transform = splashEntity->getComponent<game::components::TransformComponent>()) {
                        transform->position = explodePos;
                        transform->rotation = static_cast<float>(rand() % 360);

                        float baseScale = 0.55f + static_cast<float>(rand() % 40) / 100.0f;
                        transform->scale = { baseScale * 1.35f, baseScale * 0.7f };

                        splashEntity->addComponent(std::make_unique<game::components::PopAnimationComponent>(
                            transform->scale, sf::Vector2f(baseScale, baseScale), 10.0f));
                    }

                    auto spriteComp = std::make_unique<game::components::SpriteComponent>();
                    spriteComp->setTexture(rm.getTextureShared(baseKey));
                    splashEntity->addComponent(std::move(spriteComp));
                    splashEntity->addComponent(std::make_unique<game::components::LifespanComponent>(1.5f, true));

                    context_.spawnEntity(std::move(splashEntity));
                }
            }

            // 6. EXPLOSION LOGIC: ICE SHATTER (Sople jako encje)
            if (proj->getStatusEffect() == game::components::StatusEffect::IceShatter)
            {
                auto iceEntity = std::make_unique<game::entities::Entity>();
                if (auto* transform = iceEntity->getComponent<game::components::TransformComponent>()) {
                    transform->position = explodePos;
                }

                // Dodajemy komponent AoE (Lod zadaje 40 DPS i spowalnia o 0.4x)
                auto iceAoE = std::make_unique<game::components::AoEComponent>(
                    120.0f, sf::Color(100, 200, 255, 80), 40.0f, false, 0.0f, true, 0.4f);
                iceAoE->isVisible = false;
                iceEntity->addComponent(std::move(iceAoE));

                // Czas zycia z wygaszaniem (3 sekundy)
                iceEntity->addComponent(std::make_unique<game::components::LifespanComponent>(3.0f, true));

                

                context_.spawnEntity(std::move(iceEntity));

                auto& rm = game::core::ResourceManager::get();
                std::shared_ptr<sf::Texture> shardTex = rm.hasTexture("hazard_icicle_shard") ? rm.getTextureShared("hazard_icicle_shard") : nullptr;

                std::random_device rd; std::mt19937 gen(rd());
                std::uniform_real_distribution<float> angleDist(0.f, 3.14159f * 2.f);
                std::uniform_real_distribution<float> speedDist(400.f, 750.f);

                for (int s = 0; s < 6; ++s)
                {
                    float angle = angleDist(gen);
                    sf::Vector2f dir(std::cos(angle), std::sin(angle));

                    auto shardEntity = std::make_unique<game::entities::Entity>();
                    if (auto* t = shardEntity->getComponent<game::components::TransformComponent>()) t->position = explodePos;

                    auto shardProj = std::make_unique<game::components::ProjectileComponent>(explodePos, dir);
                    shardProj->setAppearance(12.0f, sf::Color(150, 220, 255));
                    shardProj->setFriendly(proj->getIsFriendly());
                    shardProj->setDamage(actualDamage * 0.25f);
                    shardProj->setSplashKeyBase("");

                    sf::Vector2f targetPos = explodePos + (dir * 180.0f);
                    shardProj->setupParabolic(explodePos, targetPos, speedDist(gen));

                    if (shardTex) {
                        auto size = shardTex->getSize();
                        shardProj->setAnimation(shardTex, 1, 1.0f, { static_cast<int>(size.x), static_cast<int>(size.y) });
                        shardProj->setSpriteScale(3.0f, 3.0f);
                        shardProj->setWobble(false, true);
                    }

                    shardEntity->addComponent(std::move(shardProj));
                    context_.spawnEntity(std::move(shardEntity));
                }
            }


            if (!proj->getImpactSound().empty())
            {
                game::core::AudioManager::get().playSoundVolume(proj->getImpactSound(), 75.f);
            }

            entities.erase(entities.begin() + i);
        }
    }

    void CombatSystem::processEnemyDeaths(game::systems::EvolutionManager& evolutionManager)
    {
        for (int i = static_cast<int>(enemies_.size()) - 1; i >= 0; --i)
        {
            if (enemies_[i]->isDead())
            {
                if (auto* dnaComp = enemies_[i]->getComponent<game::components::DNAComponent>())
                {
                    auto* enemies_transform = enemies_[i]->getComponent<game::components::TransformComponent>();
                    if (!enemies_transform) continue;

                    const auto& dnaData = dnaComp->getDNA();
                    evolutionManager.onEnemyDeath(dnaData);

                    // Wspólne generatory losowoœci dla wszystkich dropów (efekt fontanny)
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> velXDist(-250.f, 250.f); // Rozrzut na boki
                    std::uniform_real_distribution<float> velYDist(-500.f, -200.f); // Wystrza³ w górê

                    // --- 1. ZWYK£A BIOMASA (SOK) ---
                    float randomRoll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    if (randomRoll <= dnaData.dropChance)
                    {
                        // Obliczamy CA£KOWIT¥ wartoœæ soku i od razu robimy z niej int
                        int totalJuiceInt = static_cast<int>(std::round(dnaData.baseJuice * dnaData.sizeScale));
                        if (totalJuiceInt <= 0) totalJuiceInt = 1; // Zawsze minimum 1 soku

                        // Maksymalnie 10 kropelek wizualnych, ¿eby nie lagowaæ gry przy silnych wrogach
                        int numJuiceDrops = std::min(totalJuiceInt, 10);

                        // Dzielimy wartoœæ ca³kowit¹ na kropelki (bez u³amków)
                        int baseValuePerDrop = totalJuiceInt / numJuiceDrops;
                        int remainder = totalJuiceInt % numJuiceDrops; // Reszta z dzielenia

                        for (int j = 0; j < numJuiceDrops; ++j)
                        {
                            // Jeœli jest jakaœ reszta, pierwsze kilka kropel dostaje +1 do wartoœci
                            float dropValue = static_cast<float>(baseValuePerDrop + (j < remainder ? 1 : 0));

                            sf::Vector2f randomVel = { velXDist(gen), velYDist(gen) };
                            auto juiceEntity = std::make_unique<game::entities::Entity>();

                            if (auto* transform = juiceEntity->getComponent<game::components::TransformComponent>()) {
                                transform->position = enemies_transform->position;
                            }

                            juiceEntity->addComponent(std::make_unique<game::components::JuiceComponent>(
                                dropValue, randomVel, false
                                ));

                            context_.spawnEntity(std::move(juiceEntity));
                        }
                    }

                    // --- 2. MONETY Z MUTANTÓW ---
                    if (dnaData.isMutated)
                    {
                        float coinRoll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                        if (coinRoll <= 0.07f)
                        {
                            std::uniform_int_distribution<int> coinCountDist(1, 3);
                            int numCoins = coinCountDist(gen);

                            for (int c = 0; c < numCoins; ++c)
                            {
                                sf::Vector2f randomVel = { velXDist(gen), velYDist(gen) };
                                auto coinEntity = std::make_unique<game::entities::Entity>();

                                if (auto* transform = coinEntity->getComponent<game::components::TransformComponent>()) {
                                    transform->position = enemies_transform->position;
                                    transform->scale = { 1.5f, 1.5f };
                                }

                                auto coinComp = std::make_unique<game::components::JuiceComponent>(
                                    1.0f, randomVel, true
                                    );

                                if (coinComp->glowSprite) {
                                    coinComp->glowSprite->setColor(sf::Color(255, 215, 0, 180));
                                }

                                coinEntity->addComponent(std::move(coinComp));
                                context_.spawnEntity(std::move(coinEntity));
                            }
                        }
                    }
                }

                enemies_.erase(enemies_.begin() + i);
            }
        }
    }

    void CombatSystem::processAoE(
        game::entities::Entity* player,
        float deltaTime)
    {
        auto& entities = context_.entities;

        for (auto& entity : entities)
        {
            auto* aoe =
                entity->getComponent<game::components::AoEComponent>();

            auto* aoeTransform =
                entity->getComponent<game::components::TransformComponent>();

            if (!aoe || !aoeTransform)
                continue;

            //
            // WROGOWIE
            //
            for (auto& enemy : enemies_)
            {
                if (enemy->isDead())
                    continue;

                auto* enemyTransform =
                    enemy->getComponent<game::components::TransformComponent>();

                auto* enemyStats =
                    enemy->getComponent<game::components::StatsComponent>();

                if (!enemyTransform || !enemyStats)
                    continue;

                sf::Vector2f diff =
                    enemyTransform->position -
                    aoeTransform->position;

                float distSq =
                    diff.x * diff.x +
                    diff.y * diff.y;

                if (distSq <= aoe->radius * aoe->radius)
                {
                    if (aoe->dps > 0.0f)
                    {
                        enemyStats->takeDamage(
                            aoe->dps * deltaTime
                        );
                    }

                    if (aoe->appliesSlow)
                    {
                        enemyStats->addEffect(
                            game::components::StatusType::Slow,
                            0.15f,
                            aoe->slowMultiplier
                        );
                    }
                }
            }

            //
            // GRACZ
            //
            if (!aoe->isFriendly && player && !player->isDead())
            {
                auto* playerTransform =
                    player->getComponent<game::components::TransformComponent>();

                auto* playerStats =
                    player->getComponent<game::components::StatsComponent>();

                if (playerTransform && playerStats)
                {
                    sf::Vector2f diff =
                        playerTransform->position -
                        aoeTransform->position;

                    float distSq =
                        diff.x * diff.x +
                        diff.y * diff.y;

                    if (distSq <= aoe->radius * aoe->radius)
                    {
                        if (aoe->dps > 0.0f)
                        {
                            playerStats->takeDamage(
                                aoe->dps * deltaTime
                            );
                        }

                        if (aoe->appliesSlow)
                        {
                            playerStats->addEffect(
                                game::components::StatusType::Slow,
                                0.15f,
                                aoe->slowMultiplier
                            );
                        }
                    }
                }
            }
        }
    }
}