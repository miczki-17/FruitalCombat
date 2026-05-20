#pragma once
#include "Component.h"

namespace game::components
{
    class StatsComponent : public Component
    {
    public:
        int hp = 100;
        int maxHp = 100;
        float attackSpeed = 1.0f;

        StatsComponent(int h, float atkSpd)
            : hp(h), maxHp(h), attackSpeed(atkSpd) {
        }
    };
}