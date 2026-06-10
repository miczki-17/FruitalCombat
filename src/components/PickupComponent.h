#pragma once
#include "Component.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

namespace game::components
{
    class PickupComponent final : public Component
    {
    public:
        sf::Vector2f velocity;
        bool spawning = true;
        bool isCollected = false;

        float animationTimer = 0.0f;
        float spawnScale = 0.0f;

        std::optional<sf::Sprite> sprite;
        std::optional<sf::Sprite> glowSprite;

        // Konstruktor przyjmuje klucz tekstury i pr?dko?? wyskoku
        PickupComponent(const std::string& textureKey, const sf::Vector2f& initialVelocity, bool useGlow = false);

        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}