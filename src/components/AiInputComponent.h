#pragma once
#include "Component.h"
#include "../entities/Entity.h"
#include "../genetics/DNA.h"
#include "AbilityComponent.h"
#include <cmath>
#include <random>

namespace game::components
{
    class AiInputComponent : public Component
    {
    private:
        game::entities::Entity* targetPlayer;
        game::genetics::AiBehavior behavior;

        float currentSpeed;
        float acceleration = 1800.0f;
        float drag = 5.0f;

        // Decyzyjno?? AI (?eby nie kr?ci?y si? idealnie w kó?ko)
        float decisionTimer = 0.0f;
        float strafeDirection = 1.0f; // 1.0 w prawo, -1.0 w lewo

    public:
        AiInputComponent(game::entities::Entity* player, game::genetics::AiBehavior aiBehavior, float speed)
            : targetPlayer(player), behavior(aiBehavior), currentSpeed(speed) {
        }

        void update(float dt) override
        {
            if (!owner || !targetPlayer || targetPlayer->isDead) return;

            sf::Vector2f toPlayer = targetPlayer->position - owner->position;
            float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

            sf::Vector2f desiredDir(0.f, 0.f);
            if (distance > 0.001f) toPlayer /= distance;

            // AI co jaki? czas zmienia zdanie na temat tego, w któr? stron? i?? (?amanie perfekcyjnej orbity)
            decisionTimer -= dt;
            if (decisionTimer <= 0.0f) {
                decisionTimer = (rand() % 20 + 10) / 10.0f; // Od 1 do 3 sekund
                strafeDirection = (rand() % 2 == 0) ? 1.0f : -1.0f;

                // Skirmisher mo?e odpali? skill (np. Dash) podczas zmiany decyzji!
                if (behavior == game::genetics::AiBehavior::Skirmisher) {
                    if (auto* ab = owner->getComponent<AbilityComponent>()) {
                        ab->useSkill(targetPlayer->position);
                    }
                }
            }

            // --- INTELIGENTNE ZACHOWANIE RUCHOWE ---
            switch (behavior)
            {
            case game::genetics::AiBehavior::Charger:
                desiredDir = toPlayer; // Bije prosto w gracza
                break;

            case game::genetics::AiBehavior::Sniper:
                if (distance > 450.0f) {
                    desiredDir = toPlayer;
                }
                else if (distance < 300.0f) {
                    desiredDir = -toPlayer; // Ucieka!
                }
                else {
                    // Zamiast idealnego stania, delikatnie orbituje w losowym kierunku
                    desiredDir = { -toPlayer.y * strafeDirection, toPlayer.x * strafeDirection };
                    desiredDir = desiredDir * 0.8f + toPlayer * 0.2f; // Lekko zacie?nia kr?g
                }
                break;

            case game::genetics::AiBehavior::Skirmisher:
                if (distance > 200.0f) {
                    desiredDir = toPlayer;
                }
                else {
                    // Atakuje z doskoku, po czym b?yskawicznie ucieka na boki
                    desiredDir = { -toPlayer.y * strafeDirection, toPlayer.x * strafeDirection };
                    desiredDir -= toPlayer * 0.5f;
                }
                break;
            }

            // Normalizacja ostatecznego wektora
            float dirLen = std::sqrt(desiredDir.x * desiredDir.x + desiredDir.y * desiredDir.y);
            if (dirLen > 0.001f) desiredDir /= dirLen;

            // --- APLIKACJA FIZYKI ---
            owner->isMoving = (std::abs(desiredDir.x) > 0.1f || std::abs(desiredDir.y) > 0.1f);
            if (toPlayer.x > 0.1f)       owner->facingRight = true; // Zawsze patrzy na gracza, nawet jak ucieka!
            else if (toPlayer.x < -0.1f) owner->facingRight = false;

            owner->velocity += desiredDir * acceleration * dt;
            owner->velocity -= owner->velocity * drag * dt;

            float currentVelSq = owner->velocity.x * owner->velocity.x + owner->velocity.y * owner->velocity.y;
            if (currentVelSq > currentSpeed * currentSpeed && !owner->isRolling && owner->actionTimer <= 0.0f) {
                float actualSpeed = std::sqrt(currentVelSq);
                owner->velocity = (owner->velocity / actualSpeed) * currentSpeed;
            }

            // --- WALKA (Poci?gni?cie za spust) ---
            // Je?li AI ma podpi?t? bro?, próbuje strzela? prosto w gracza, o ile jest w zasi?gu (np. 600 px)
            if (distance < 600.0f) {
                if (auto* ab = owner->getComponent<AbilityComponent>()) {
                    ab->useWeapon(targetPlayer->position);
                }
            }
        }
    };
}