#pragma once
#include "Component.h"

namespace game::components
{
    // Komponent-flaga. Je?li byt go ma, wybuchnie po ?mierci.
    class DeathRattleComponent final : public Component
    {
    public:
        float radius = 150.0f;
        float dps = 20.0f;
        float duration = 5.0f;
        std::string textureKey;

        DeathRattleComponent(float r, float d, float dur, const std::string& texKey = "")
            : radius(r), dps(d), duration(dur), textureKey(texKey) {
        }
    };
}