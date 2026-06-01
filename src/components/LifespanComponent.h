// --- LifespanComponent.h ---

#pragma once
#include "Component.h"

namespace game::components
{
    class LifespanComponent final : public Component
    {
    public:
        float lifetime;
        float maxLifetime;
        bool fadeOut;

        LifespanComponent(float duration, bool fade = false);
        void update(float dt) override;
    };
}