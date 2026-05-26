// --- PlayerInputComponent.h ---

#pragma once
#include "Component.h"

namespace game
{
    class Game;
}

namespace game::components
{
    class PlayerInputComponent final : public Component
    {
    public:
        explicit PlayerInputComponent(game::Game* game);
        void update(float dt) override;

    private:
        game::Game* game_;
    };
}