// --- WindupBruisierAbility.cpp ---

#include "WindupBruisierAbility.h"
#include "../components/TransformComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/StatsComponent.h"
#include "../components/ParticleComponent.h"
#include "../components/LifespanComponent.h"
#include "../components/MovementComponent.h"
#include <cmath>
#include <random>

namespace game::components
{
    WindupBruiserAbility::WindupBruiserAbility(
        game::ArenaContext* context,
        game::entities::Entity* owner,
        game::entities::Entity* targetPlayer,
        const nlohmann::json& config,
        std::string sourceName)
        : Ability(std::move(sourceName)),
        context_(context), owner_(owner), targetPlayer_(targetPlayer)
    {
        triggerDistance_ = config.value("triggerDistance", 130.0f);
        signalTime_ = config.value("signalTime", 0.4f);
        jumpHeight_ = config.value("jumpHeight", 80.0f);
        jumpSpeed_ = config.value("jumpSpeed", 300.0f);
        fallSpeed_ = config.value("fallSpeed", 800.0f);
        recoveryTime_ = config.value("recoveryTime", 0.8f);
        damageRadius_ = config.value("damageRadius", 90.0f);
        damage_ = config.value("damage", 40.0f);
    }

    void WindupBruiserAbility::execute(
        const sf::Vector2f& origin,
        const sf::Vector2f& targetPosition,
        const sf::Vector2f& ownerVelocity)
    {
        if (state_ != BruiserState::Idle) return;

        auto* transform = owner_->getComponent<TransformComponent>();
        if (!transform) return;

        state_ = BruiserState::Signaling;
        timer_ = signalTime_;
        originalPosition_ = transform->position;
        transform->actionTimer = 10.0f;
    }

    void WindupBruiserAbility::update(float dt)
    {
        if (!owner_ || owner_->isDead() || !targetPlayer_) return;

        auto* transform = owner_->getComponent<TransformComponent>();
        auto* spriteComp = owner_->getComponent<SpriteComponent>();
        auto* playerTrans = targetPlayer_->getComponent<TransformComponent>();

        if (!transform || !spriteComp || !playerTrans) return;

        if (state_ == BruiserState::Idle)
        {
            sf::Vector2f diff = playerTrans->position - transform->position;
            float distSq = (diff.x * diff.x) + (diff.y * diff.y);

            if (distSq <= (triggerDistance_ * triggerDistance_))
            {
                execute(transform->position, playerTrans->position, { 0, 0 });
            }
            return;
        }

        if (auto* moveComp = owner_->getComponent<MovementComponent>()) {
            moveComp->setDesiredDirection({ 0.0f, 0.0f });
        }

        switch (state_)
        {
        case BruiserState::Signaling:
        {
            timer_ -= dt;
            spriteComp->setCustomScale(1.0f);
            spriteComp->setTint(sf::Color(255, 80, 80));

            float shakeOffset = std::sin(timer_ * 80.0f) * 5.0f;
            transform->position = originalPosition_ + sf::Vector2f(shakeOffset, 0.0f);

            if (timer_ <= 0.0f) {
                transform->position = originalPosition_;

                sf::Vector2f diff = playerTrans->position - transform->position;
                float timeInAir = (jumpHeight_ / jumpSpeed_) + (jumpHeight_ / fallSpeed_);

                leapVelocity_ = diff / timeInAir;

                float speedSq = (leapVelocity_.x * leapVelocity_.x) + (leapVelocity_.y * leapVelocity_.y);
                if (speedSq > 600.0f * 600.0f) {
                    float length = std::sqrt(speedSq);
                    leapVelocity_ = (leapVelocity_ / length) * 600.0f;
                }

                state_ = BruiserState::JumpingUp;
            }
            break;
        }

        case BruiserState::JumpingUp:
        {
            transform->zOffset -= jumpSpeed_ * dt;
            transform->position += leapVelocity_ * dt;

            if (transform->zOffset <= -jumpHeight_) {
                transform->zOffset = -jumpHeight_;
                state_ = BruiserState::FallingDown;
            }
            break;
        }

        case BruiserState::FallingDown:
        {
            transform->zOffset += fallSpeed_ * dt;
            transform->position += leapVelocity_ * dt;

            if (transform->zOffset >= 0.0f)
            {
                transform->zOffset = 0.0f;
                spriteComp->setCustomScale(0.8f);

                sf::Vector2f finalDiff = playerTrans->position - transform->position;
                if ((finalDiff.x * finalDiff.x + finalDiff.y * finalDiff.y) <= (damageRadius_ * damageRadius_))
                {
                    if (auto* playerStats = targetPlayer_->getComponent<StatsComponent>()) {
                        playerStats->takeDamage(damage_, sourceName_);
                        if (auto* pSprite = targetPlayer_->getComponent<SpriteComponent>()) {
                            pSprite->triggerHitFlash();
                        }
                    }
                }

                std::random_device rd; std::mt19937 gen(rd());
                std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f);
                std::uniform_real_distribution<float> speedDist(230.0f, 450.0f);

                for (int i = 0; i < 15; ++i) {
                    float angle = angleDist(gen);
                    float spd = speedDist(gen);
                    auto dust = std::make_unique<game::entities::Entity>();
                    auto dustTrans = std::make_unique<game::components::TransformComponent>(transform->position);
                    dustTrans->velocity = sf::Vector2f(std::cos(angle) * spd, std::sin(angle) * spd);
                    dust->addComponent(std::move(dustTrans));
                    dust->addComponent(std::make_unique<game::components::LifespanComponent>(0.5f, true));
                    dust->addComponent(std::make_unique<game::components::ParticleComponent>(
                        6.0f, context_->mapDustColor, 8.0f));

                    context_->spawnEntity(std::move(dust));
                }

                state_ = BruiserState::Recovering;
                timer_ = recoveryTime_;
            }
            break;
        }

        case BruiserState::Recovering:
        {
            timer_ -= dt;
            spriteComp->setTint(sf::Color(120, 120, 120));

            if (timer_ <= 0.0f) {
                spriteComp->setTint(sf::Color::White);
                transform->actionTimer = 0.0f;
                state_ = BruiserState::Idle;
            }
            break;
        }
        default: break;
        }
    }
}