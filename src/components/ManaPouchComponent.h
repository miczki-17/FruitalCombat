#pragma once
#include "Component.h"

namespace game::components
{
    class ManaPouchComponent final : public Component
    {
    public:
        float manaAmount;

        explicit ManaPouchComponent(float mana = 40.0f)
            :  manaAmount(mana){
        }
    };
}