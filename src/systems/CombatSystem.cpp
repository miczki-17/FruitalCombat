// --- CombatSystem.cpp ---

#include "CombatSystem.h"
#include "../core/Game.h"
#include "../core/ArenaContext.h"
#include "../entities/Entity.h"
#include "../components/StatsComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/DNAComponent.h"
#include "EvolutionManager.h"
#include <algorithm>
#include <cstdlib>
#include <string>

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

    void CombatSystem::processBulletDamage()
    {
        float defaultBulletDamage = 25.0f;
        auto& bullets = context_.bullets;

        for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; --i)
        {
            if (!bullets[i].getIsActive())
            {
                sf::Vector2f explodePos = bullets[i].getPosition();
                float explosionRadius = 70.0f;

				// 1. area damage logic
                for (auto& enemy : enemies_)
                {
                    if (enemy->isDead) continue;

                    sf::Vector2f diff = explodePos - enemy->position;

                    if (diff.lengthSquared() < (explosionRadius * explosionRadius))
                    {
                        if (auto* stats = enemy->getComponent<game::components::StatsComponent>())
                        {
                            stats->takeDamage(defaultBulletDamage);

                            if (auto* sprite = enemy->getComponent<game::components::SpriteComponent>())
                            {
                                sprite->triggerHitFlash();
                            }

							// Dynamiic damage numbers floating text
                            context_.floatingTexts.emplace_back(
                                game_->mainFont,
                                "-" + std::to_string(static_cast<int>(defaultBulletDamage)),
                                enemy->position,
                                sf::Color::Red
                            );
                        }
                    }
                }

				// 2. Creaing acid splash visual effect at the explosion site
                if (!context_.splashTextures.empty())
                {
                    int randomTexIdx = std::rand() % context_.splashTextures.size();
                    context_.acidSplashes.emplace_back(explodePos, context_.splashTextures[randomTexIdx]);
                }

				// 3. Creating AoE zone if the bullet has a poison status effect
                if (bullets[i].getStatusEffect() == game::components::StatusEffect::Poison)
                {
                    AoEZone puddle;
                    puddle.radius = 65.0f;
                    puddle.dps = 15.0f;
                    puddle.shape.setRadius(puddle.radius);
                    puddle.shape.setOrigin({ puddle.radius, puddle.radius });
                    puddle.shape.setPosition(explodePos);
                    puddle.shape.setFillColor(sf::Color::Transparent);
                    puddle.shape.setOutlineThickness(0.0f);
                    puddle.lifetime = 4.0f;
                    puddle.maxLifetime = 4.0f;

                    context_.zones.push_back(puddle);
                }

                
                bullets.erase(bullets.begin() + i);
            }
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