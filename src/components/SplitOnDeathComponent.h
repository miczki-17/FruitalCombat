#pragma once
#include "Component.h"

namespace game::components
{
    class SplitOnDeathComponent final : public Component
    {
    public:
        int splitCount;
        std::string splitSkinKey;
        float splitScale;

        SplitOnDeathComponent(int count = 4, const std::string& skinKey = "", float scale = 0.5f)
            : splitCount(count), splitSkinKey(skinKey), splitScale(scale) {
        }
    };
}