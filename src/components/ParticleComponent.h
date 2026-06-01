#pragma once
#include "Component.h"
#include <SFML/Graphics/Color.hpp>

namespace game::components
{
    class ParticleComponent final : public Component
    {
    public:
        float size;
        sf::Color baseColor;
        float friction; // Si³a hamowania

        ParticleComponent(float s, sf::Color c, float f = 5.0f);

        void update(float dt) override;
    };
}