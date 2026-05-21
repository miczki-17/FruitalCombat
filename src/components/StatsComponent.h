#pragma once
#include "Component.h"
#include "../entities/Entity.h"

namespace game::components
{
    class StatsComponent : public Component
    {
    public:
        float hp;
        float maxHp;
        float attackSpeed;

        StatsComponent(float maxHealth, float atkSpd)
            : hp(maxHealth), maxHp(maxHealth), attackSpeed(atkSpd) {
        }

        // Decreases HP and marks the entity as dead if it drops to 0
        void takeDamage(float amount)
        {
            hp -= amount;
            if (hp <= 0.0f) {
                hp = 0.0f;
                if (owner) owner->isDead = true;
            }
        }

        // Helper to get HP percentage (0.0 to 1.0) for health bars
        float getHpPercentage() const
        {
            if (maxHp <= 0.0f) return 0.0f;
            return hp / maxHp;
        }
    };
}