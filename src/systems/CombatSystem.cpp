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
        if (!player || player->isDead) return;

        auto& drops = context_.juiceDrops;

        for (int i = static_cast<int>(drops.size()) - 1; i >= 0; --i)
        {
            // check if player is close enough to collect the juice drop
            if (drops[i].isCollected)
            {
                float juiceValue = drops[i].value;
                game_->profile.addJuice(static_cast<int>(juiceValue));

                if (auto* stats = player->getComponent<game::components::StatsComponent>())
                {
                    stats->addUltCharge(juiceValue * 0.5f);
                }

                // safety check to prevent out-of-range access, should never trigger due to loop structure
                drops.erase(drops.begin() + i);
            }
        }
    }

    void CombatSystem::processBulletDamage(game::entities::Entity* player)
    {
        auto& bullets = context_.bullets;
        std::vector<game::components::Bullet> newShards;

        for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; --i)
        {
            if (!bullets[i].getIsActive())
            {
                sf::Vector2f explodePos = bullets[i].getPosition();
                float explosionRadius = 70.0f;
                float actualDamage = bullets[i].getDamage();

                // 1. AREA DAMAGE: WROGOWIE
                for (auto& enemy : enemies_)
                {
                    if (enemy->isDead) continue;
                    sf::Vector2f diff = explodePos - enemy->position;

                    if (diff.lengthSquared() < (explosionRadius * explosionRadius))
                    {
                        if (auto* stats = enemy->getComponent<game::components::StatsComponent>())
                        {
                            stats->takeDamage(actualDamage);
                            if (auto* sprite = enemy->getComponent<game::components::SpriteComponent>()) {
                                sprite->triggerHitFlash();
                            }
                            context_.floatingTexts.emplace_back(
                                game_->mainFont, "-" + std::to_string(static_cast<int>(actualDamage)),
                                enemy->position, sf::Color::Red
                            );
                        }
                    }
                }

                // 2. AREA DAMAGE: GRACZ (Je?li pocisk by? wrogi!)
                if (!bullets[i].getIsFriendly() && player && !player->isDead)
                {
                    sf::Vector2f diffToPlayer = explodePos - player->position;
                    if (diffToPlayer.lengthSquared() < (explosionRadius * explosionRadius))
                    {
                        if (auto* stats = player->getComponent<game::components::StatsComponent>())
                        {
                            stats->takeDamage(actualDamage);
                            if (auto* sprite = player->getComponent<game::components::SpriteComponent>()) {
                                sprite->triggerHitFlash();
                            }
                            context_.floatingTexts.emplace_back(
                                game_->mainFont, "-" + std::to_string(static_cast<int>(actualDamage)),
                                player->position, sf::Color::Red
                            );
                        }
                    }
                }

                // 3. WIZUALNY EFEKT ROZPRYSKU (SPLASH)
                if (bullets[i].getStatusEffect() != game::components::StatusEffect::IceShatter &&
                    bullets[i].getStatusEffect() != game::components::StatusEffect::SporePoison)
                {
                    std::string baseKey = bullets[i].getSplashKeyBase();
                    if (!baseKey.empty())
                    {
                        std::string splashKey = baseKey + "_" + std::to_string((std::rand() % 3) + 1);
                        auto& rm = game::core::ResourceManager::get();

                        if (rm.hasTexture(splashKey)) {
                            context_.acidSplashes.emplace_back(explodePos, rm.getTextureShared(splashKey));
                        }
                    }
                }

                // 4. AOE DLA TRUCIZNY 
                if (bullets[i].getStatusEffect() == game::components::StatusEffect::Poison)
                {
                    AoEZone puddle;
                    puddle.radius = 80.0f;
                    puddle.dps = 25.0f;
                    puddle.shape.setRadius(puddle.radius);
                    puddle.shape.setOrigin({ puddle.radius, puddle.radius });
                    puddle.shape.setPosition(explodePos);
                    puddle.shape.setFillColor(sf::Color(50, 205, 50, 100)); // Zielona, toksyczna chmura
                    puddle.shape.setOutlineThickness(0.0f);
                    puddle.lifetime = 5.0f;
                    puddle.maxLifetime = 5.0f;
                    puddle.appliesPoison = true;

                    context_.zones.push_back(puddle);
                }

                if (bullets[i].getStatusEffect() == game::components::StatusEffect::SporePoison)
                {
                    // A) TWORZYMY STREF? (Wi?ksza ka?u?a)
                    AoEZone puddle;
                    puddle.radius = 120.0f;
                    puddle.dps = 5.0f;
                    puddle.shape.setRadius(puddle.radius);
                    puddle.shape.setOrigin({ puddle.radius, puddle.radius });
                    puddle.shape.setPosition(explodePos);
                    puddle.shape.setFillColor(sf::Color(50, 205, 50, 130)); // Toksyczna ziele?, lekko ciemniejsza
                    puddle.shape.setOutlineThickness(0.0f);
                    puddle.lifetime = 6.0f;
                    puddle.maxLifetime = 6.0f;
                    puddle.appliesPoison = true;

                    context_.zones.push_back(puddle);

                    std::string baseKey = bullets[i].getSplashKeyBase();
                    if (!baseKey.empty())
                    {
                        std::string splashKey = baseKey;
                        auto& rm = game::core::ResourceManager::get();

                        if (rm.hasTexture(splashKey)) {
                            context_.acidSplashes.emplace_back(explodePos, rm.getTextureShared(splashKey));
                        }
                    }
                }

                // 5. EXPLOSION LOGIC: ICE SHATTER (Sopel lodowy z Lodówki)
                if (bullets[i].getStatusEffect() == game::components::StatusEffect::IceShatter)
                {
                    AoEZone iceZone;
                    iceZone.radius = 120.0f;
                    iceZone.shape.setRadius(120.0f);
                    iceZone.shape.setOrigin({ 120.0f, 120.0f });
                    iceZone.shape.setPosition(explodePos);
                    iceZone.shape.setFillColor(sf::Color(100, 200, 255, 80));
                    iceZone.lifetime = 3.0f;
                    iceZone.maxLifetime = 3.0f;
                    iceZone.dps = 40.0f;
                    iceZone.appliesSlow = true;
                    iceZone.slowMultiplier = 0.4f;

                    context_.zones.push_back(iceZone);

                    auto& rm = game::core::ResourceManager::get();
                    std::shared_ptr<sf::Texture> shardTex = nullptr;
                    if (rm.hasTexture("hazard_icicle_shard")) {
                        shardTex = rm.getTextureShared("hazard_icicle_shard");
                    }

                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> angleDist(0.f, 3.14159f * 2.f);
                    std::uniform_real_distribution<float> speedDist(400.f, 750.f);

                    for (int s = 0; s < 6; ++s)
                    {
                        float angle = angleDist(gen);
                        sf::Vector2f dir(std::cos(angle), std::sin(angle));

                        game::components::Bullet shard(explodePos, dir);
                        shard.setAppearance(12.0f, sf::Color(150, 220, 255)); // Kolizja zwi?kszona do 12

                        // Od?amki dziedzicz? to, czy rani? gracza, z g?ównego sopla
                        shard.setFriendly(bullets[i].getIsFriendly());
                        shard.setDamage(actualDamage * 0.25f);
                        shard.setSplashKeyBase(""); // Od?amki nie bryzgaj? mazi? przy upadku

                        sf::Vector2f targetPos = explodePos + (dir * 180.0f);
                        shard.setupParabolic(explodePos, targetPos, speedDist(gen));

                        if (shardTex) {
                            auto size = shardTex->getSize();
                            shard.setAnimation(shardTex, 1, 1.0f, { static_cast<int>(size.x), static_cast<int>(size.y) });
                            shard.setSpriteScale(3.0f, 3.0f); // Grafika od?amków x3
                            shard.setWobble(false, true);
                        }

                        newShards.push_back(shard);
                    }
                }

                // Sounds
                if (!bullets[i].getImpactSound().empty())
                {
                    game::core::AudioManager::get().playSoundVolume(bullets[i].getImpactSound(), 75.f);
                }

                bullets.erase(bullets.begin() + i);
            }
        }

        // Dodajemy nowo powsta?e od?amki z powrotem do g?ównej puli pocisków areny
        for (const auto& shard : newShards) {
            bullets.push_back(shard);
        }
    }

    void CombatSystem::processEnemyDeaths(game::systems::EvolutionManager& evolutionManager)
    {
        for (int i = static_cast<int>(enemies_.size()) - 1; i >= 0; --i)
        {
            if (auto* stats = enemies_[i]->getComponent<game::components::StatsComponent>())
            {
                if (stats->getHealth() <= 0.0f)
                {
                    enemies_[i]->isDead = true;
                }
            }

            if (enemies_[i]->isDead)
            {
                if (auto* dnaComp = enemies_[i]->getComponent<game::components::DNAComponent>())
                {
                    const auto& dnaData = dnaComp->getDNA();

                    // genetic algorithm feeding
                    evolutionManager.onEnemyDeath(dnaData);

                    // random chance to drop juice based on enemy's DNA dropChance
                    float randomRoll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    if (randomRoll <= dnaData.dropChance)
                    {
                        context_.juiceDrops.emplace_back(enemies_[i]->position, dnaData.baseJuice * dnaData.sizeScale);
                    }
                }

                // Removing dead enemy from the game loop
                enemies_.erase(enemies_.begin() + i);
            }
        }
    }
}