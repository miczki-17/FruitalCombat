#pragma once
#include "Component.h"

namespace game::components
{
    class MedkitComponent final : public Component
    {
    public:
        float healAmount;

        explicit MedkitComponent(float heal = 10.0f)
            : healAmount(heal) {
        }
    };
}