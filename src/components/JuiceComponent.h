#pragma once
#include "Component.h"

namespace game::components
{
    class JuiceComponent final : public Component
    {
    public:
        float value;
        bool isCoin;

        explicit JuiceComponent(float val, bool coin = false)
            : value(val), isCoin(coin) {
        }
    };
}