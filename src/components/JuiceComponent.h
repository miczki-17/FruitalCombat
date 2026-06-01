// --- JuiceComponent.h ---

#pragma once
#include "Component.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <optional>

namespace game::components
{
    class JuiceComponent final : public Component
    {
    public:
        float value;
        bool isCollected = false;

        std::optional<sf::Sprite> sprite;
        std::optional<sf::Sprite> glowSprite;

        float animationTimer = 0.f;
        sf::Vector2f velocity;
        float spawnScale = 0.55f;
        bool spawning = true;

        // Konstruktor przyjmuje wartosc i pocz¹tkowa prêdkosc dropu z wroga
        JuiceComponent(float xpValue, const sf::Vector2f& initialVelocity);

        void update(float dt) override;
        void render(sf::RenderWindow& window);
    };
}