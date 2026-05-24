#include "RindRollAbility.h"
#include "../entities/Entity.h"
#include "../components/StatsComponent.h"
#include <cmath>
#include <random>

namespace game::components
{
    RindRollAbility::RindRollAbility(game::entities::Entity* targetEntity, game::ArenaContext* ctx, std::vector<std::unique_ptr<game::entities::Entity>>* targetsList, float kRadius, float kForce)
        : entity(targetEntity), context(ctx), enemies(targetsList), singleTarget(nullptr), knockbackRadius(kRadius), knockbackForce(kForce) {}

    RindRollAbility::RindRollAbility(game::entities::Entity* targetEntity, game::ArenaContext* ctx, game::entities::Entity* playerTarget, float kRadius, float kForce)
        : entity(targetEntity), context(ctx), enemies(nullptr), singleTarget(playerTarget), knockbackRadius(kRadius), knockbackForce(kForce) {}

    void RindRollAbility::update(float dt)
    {
        if (currentTimer > 0.0f) currentTimer -= dt;

        if (entity != nullptr && entity->isRolling && entity->actionTimer > 0.0f)
        {
            // 1. CZ¥STECZKI KURZU
            if (context != nullptr)
            {
                std::random_device rd; std::mt19937 gen(rd());
                std::uniform_real_distribution<float> offset(-15.0f, 15.0f);
                std::uniform_real_distribution<float> velSpread(-30.0f, 30.0f);

                sf::Vector2f backDir = -entity->velocity;
                float length = std::sqrt(backDir.x * backDir.x + backDir.y * backDir.y);
                if (length > 0.001f) backDir /= length;

                for (int i = 0; i < 4; ++i) {
                    context->walkParticles.emplace_back(
                        entity->position + sf::Vector2f(offset(gen), offset(gen) + 15.0f),
                        (backDir * 150.0f) + sf::Vector2f(velSpread(gen), velSpread(gen)),
                        0.4f, 5.0f, sf::Color(255, 120, 20, 220)
                    );
                }
            }

            // 2. OBRA¯ENIA I PROMIEÑ ODRZUTU (Dla Gracza)
            if (enemies != nullptr)
            {
                for (auto& enemy : *enemies)
                {
                    if (enemy->isDead) continue;
                    sf::Vector2f diff = enemy->position - entity->position;
                    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                    // Sprawdzamy czy wróg jest w promieniu odrzutu
                    if (dist < knockbackRadius)
                    {
                        // Im bli¿ej œrodka, tym mocniejszy efekt (od 1.0 w œrodku do 0.0 na krawêdzi)
                        float effectMulti = 1.0f - (dist / knockbackRadius);

                        if (auto* stats = enemy->getComponent<StatsComponent>()) {
                            stats->takeDamage(60.0f * effectMulti * dt);
                        }

                        // Zastosowanie si³y odrzutu (Knockback)
                        if (dist > 0.001f) {
                            enemy->position += (diff / dist) * (knockbackForce * effectMulti) * dt;
                        }
                    }
                }
            }
            // 3. OBRA¯ENIA I PROMIEÑ ODRZUTU (Dla Mutanta uderzaj¹cego w Gracza)
            else if (singleTarget != nullptr && !singleTarget->isDead)
            {
                sf::Vector2f diff = singleTarget->position - entity->position;
                float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                if (dist < knockbackRadius) {
                    float effectMulti = 1.0f - (dist / knockbackRadius);
                    if (auto* stats = singleTarget->getComponent<StatsComponent>()) {
                        stats->takeDamage(20.0f * dt);
                    }
                    if (dist > 0.001f) {
                        singleTarget->position += (diff / dist) * (knockbackForce * effectMulti) * dt;
                    }
                }
            }
        }
    }

    void RindRollAbility::execute(const sf::Vector2f& startPos, const sf::Vector2f& targetWorldPos, const sf::Vector2f& shooterVelocity)
    {
        if (currentTimer <= 0.0f && entity != nullptr)
        {
            sf::Vector2f aimDir = targetWorldPos - startPos;
            float length = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);
            if (length > 0.001f)
            {
                aimDir /= length;
                entity->velocity = aimDir * rollSpeed;
                entity->overrideSpeedLimit = rollSpeed;
                entity->isRolling = true;
                entity->actionTimer = rollDuration;
                currentTimer = cooldown;
            }
        }
    }
}