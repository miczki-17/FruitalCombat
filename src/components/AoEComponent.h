// --- AoEComponent.h ---

#pragma once
#include "Component.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace game::components
{
    class AoEComponent final : public Component
    {
    public:
        sf::CircleShape shape;
        std::optional<sf::Sprite> sprite;

        float radius;
        float dps;
        bool appliesPoison;
        float poisonDps;
        bool appliesSlow;
        float slowMultiplier;

        bool isVisible = true;
        bool isFriendly;

        AoEComponent(float r, sf::Color color, float damagePerSec = 0.0f,
            bool poison = false, float pDps = 0.0f,
            bool slow = false, float slowMult = 1.0f,
            bool friendly = true);

        void setTexture(std::shared_ptr<sf::Texture> tex);

        void update(float dt) override;
        void render(sf::RenderWindow& window);
    };
}