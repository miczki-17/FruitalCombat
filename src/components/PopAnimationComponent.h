// --- PopAnimationController ---

#pragma once
#include "Component.h"
#include <SFML/System/Vector2.hpp>

namespace game::components
{
    class PopAnimationComponent final : public Component
    {
    public:
        sf::Vector2f startScale;
        sf::Vector2f targetScale;
        float animSpeedMult;
        float currentAnimTime = 0.0f;

        PopAnimationComponent(sf::Vector2f start, sf::Vector2f target, float speed = 10.0f)
            : startScale(start), targetScale(target), animSpeedMult(speed) {}

        void update(float dt) override;
    };
}