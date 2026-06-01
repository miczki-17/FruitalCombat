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

        auto& drops = context_.juiceDrops;

        for (int i = static_cast<int>(drops.size()) - 1; i >= 0; --i)
        {
            if (drops[i].isCollected)
            {
                float juiceValue = drops[i].value;
                game_->profile.addJuice(static_cast<int>(juiceValue));

                if (auto* stats = player->getComponent<game::components::StatsComponent>())
                {
                    stats->addUltCharge(juiceValue * 0.5f);
                }

                drops.erase(drops.begin() + i);
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
            if (proj->getStatusEffect() == game::components::StatusEffect::Poison ||
                proj->getStatusEffect() == game::components::StatusEffect::SporePoison)
            {
                AoEZone puddle;
                puddle.radius = (proj->getStatusEffect() == game::components::StatusEffect::SporePoison) ? 120.0f : 80.0f;
                puddle.dps = (proj->getStatusEffect() == game::components::StatusEffect::SporePoison) ? 5.0f : 25.0f;
                puddle.shape.setRadius(puddle.radius);
                puddle.shape.setOrigin({ puddle.radius, puddle.radius });
                puddle.shape.setPosition(explodePos);
                puddle.shape.setFillColor(sf::Color(50, 205, 50, 130));
                puddle.shape.setOutlineThickness(0.0f);
                puddle.lifetime = 5.0f;
                puddle.maxLifetime = 5.0f;
                puddle.appliesPoison = true;

                context_.zones.push_back(puddle);

                if (proj->getStatusEffect() == game::components::StatusEffect::SporePoison)
                {
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
            }

            // 5. EXPLOSION LOGIC: ICE SHATTER (Sople jako encje)
            if (proj->getStatusEffect() == game::components::StatusEffect::IceShatter)
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

                    float randomRoll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    if (randomRoll <= dnaData.dropChance)
                    {
                        context_.juiceDrops.emplace_back(enemies_transform->position, dnaData.baseJuice * dnaData.sizeScale);
                    }
                }

                enemies_.erase(enemies_.begin() + i);
            }
        }
    }
}