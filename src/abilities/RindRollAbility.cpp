#include "RindRollAbility.h"

#include "../entities/Entity.h"
#include "../components/StatsComponent.h"
#include "../components/TransformComponent.h"
#include "../components/LifespanComponent.h"
#include "../components/ParticleComponent.h"
#include "../core/ArenaContext.h"

#include <cmath>
#include <random>

namespace game::components
{
    RindRollAbility::RindRollAbility(
        game::entities::Entity* owner,
        game::ArenaContext* ctx,
        std::vector<std::unique_ptr<game::entities::Entity>>* targetsList,
        float kRadius,
        float kForce)
        : owner_(owner), context_(ctx), enemies_(targetsList), singleTarget_(nullptr), knockbackRadius_(kRadius), knockbackForce_(kForce)
    {
    }

    RindRollAbility::RindRollAbility(
        game::entities::Entity* owner,
        game::ArenaContext* ctx,
        game::entities::Entity* playerTarget,
        float kRadius,
        float kForce)
        : owner_(owner), context_(ctx), enemies_(nullptr), singleTarget_(playerTarget), knockbackRadius_(kRadius), knockbackForce_(kForce)
    {
    }

    void RindRollAbility::update(float dt)
    {
        if (currentTimer_ > 0.0f) currentTimer_ -= dt;

        if (!owner_ || !context_) return;

        auto* owner_transform = owner_->getComponent<TransformComponent>();
        if (!owner_transform) return;

        if (owner_transform->isRolling && owner_transform->actionTimer > 0.0f)
        {
            // 1. CZ¥STECZKI KURZU (Tymczasowo zostawiamy w starym systemie, to tylko efekt wizualny)
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_real_distribution<float> offset(-15.0f, 15.0f);
            std::uniform_real_distribution<float> velSpread(-30.0f, 30.0f);

            sf::Vector2f backDir = -owner_transform->velocity;
            float length = std::sqrt(backDir.x * backDir.x + backDir.y * backDir.y);
            if (length > 0.001f) backDir /= length;

            for (int i = 0; i < 4; ++i) {
                // Tworzymy now¹ encjê cz¹steczki
                auto dust = std::make_unique<game::entities::Entity>();

                // 1. Ustawiamy pozycjê (z offsetem) i prêdkoœæ (z odrzutem i rozrzutem)
                if (auto* t = dust->getComponent<game::components::TransformComponent>()) {
                    t->position = owner_transform->position + sf::Vector2f(offset(gen), offset(gen) + 15.0f);
                    t->velocity = (backDir * 150.0f) + sf::Vector2f(velSpread(gen), velSpread(gen));
                }

                // 2. Czas ¿ycia: 0.4 sekundy, p³ynne wygaszanie (fadeOut = true)
                dust->addComponent(std::make_unique<game::components::LifespanComponent>(0.4f, true));

                // 3. Rozmiar: 5.0f, Kolor: Pomarañczowy, Tarcie: 5.0f (¿eby iskry ³adnie zwalnia³y)
                dust->addComponent(std::make_unique<game::components::ParticleComponent>(
                    5.0f, sf::Color(255, 120, 20, 220), 5.0f));

                // Wrzucamy do kot³a ECS!
                context_->spawnEntity(std::move(dust));
            }

            // 2. OBRAZENIA I PROMIEN ODRZUTU (Dla Gracza walcz¹cego z grupa)
            if (enemies_ != nullptr)
            {
                for (auto& enemy : *enemies_)
                {
                    if (enemy->isDead()) continue;

                    auto* enemy_transform = enemy->getComponent<TransformComponent>();
                    if (!enemy_transform) continue; // continue

                    sf::Vector2f diff = enemy_transform->position - owner_transform->position;
                    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                    if (dist < knockbackRadius_)
                    {
                        float effectMulti = 1.0f - (dist / knockbackRadius_);

                        if (auto* stats = enemy->getComponent<StatsComponent>()) {
                            stats->takeDamage(60.0f * effectMulti * dt);
                        }

                        if (dist > 0.001f) {
                            enemy_transform->position += (diff / dist) * (knockbackForce_ * effectMulti) * dt;
                        }
                    }
                }
            }
            // 3. OBRAZENIA I PROMIEN ODRZUTU (Dla Mutanta uderzajacego w Gracza)
            else if (singleTarget_ != nullptr && !singleTarget_->isDead())
            {
                auto* singleTarget_transform = singleTarget_->getComponent<TransformComponent>();
                if (singleTarget_transform)
                {
                    sf::Vector2f diff = singleTarget_transform->position - owner_transform->position;
                    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                    if (dist < knockbackRadius_)
                    {
                        float effectMulti = 1.0f - (dist / knockbackRadius_);

                        if (auto* stats = singleTarget_->getComponent<StatsComponent>()) {
                            stats->takeDamage(20.0f * dt);
                        }

                        if (dist > 0.001f) {
                            singleTarget_transform->position += (diff / dist) * (knockbackForce_ * effectMulti) * dt;
                        }
                    }
                }
            }
        }
    }

    void RindRollAbility::execute(const sf::Vector2f& startPos, const sf::Vector2f& targetWorldPos, const sf::Vector2f& shooterVelocity)
    {
        if (currentTimer_ <= 0.0f && owner_ != nullptr)
        {
            auto* owner_transform = owner_->getComponent<TransformComponent>();
            if (!owner_transform) return;

            sf::Vector2f aimDir = targetWorldPos - startPos;
            float length = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);

            if (length > 0.001f)
            {
                aimDir /= length;
                owner_transform->velocity = aimDir * rollSpeed_;
                owner_transform->overrideSpeedLimit = rollSpeed_;
                owner_transform->isRolling = true;
                owner_transform->actionTimer = rollDuration_;
                currentTimer_ = cooldown_;
            }
        }
    }
}