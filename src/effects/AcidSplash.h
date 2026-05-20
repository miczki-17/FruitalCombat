// --- AcidSplash.h ---


#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

namespace game::effects
{
    class AcidSplash
    {
    private:

        std::optional<sf::Sprite> sprite;

        std::shared_ptr<sf::Texture> texture;

        bool active = true;

        float lifetime = 0.0f;
        float maxLifetime = 1.5f;

        sf::Vector2f targetScale;
        sf::Vector2f startScale;

    public:

        AcidSplash(
            sf::Vector2f pos,
            std::shared_ptr<sf::Texture> tex);

        void update(float dt);

        void render(sf::RenderWindow& window);

        bool isActive() const;
    };
}