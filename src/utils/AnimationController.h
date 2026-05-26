// --- AnimationController.h ---

#pragma once
#include <SFML/Graphics.hpp>

namespace game::components
{
    class AnimationController
    {
    private:
        sf::Texture* idleTexture = nullptr;
        sf::Texture* walkTexture = nullptr;

        const sf::Vector2i frameSize = { 96, 96 };

        int currentFrame = 0;
        float animationTimer = 0.0f;

        int totalIdleFrames = 4;
        int totalWalkFrames = 4;

        bool isMoving = false;
        bool facingRight = true;

    public:
        AnimationController() = default;

        void setTextures(sf::Texture& idle, int idleFrames, sf::Texture& walk, int walkFrames);

        const sf::Texture& getDefaultTexture() const;

        void setMovementState(bool moving, bool right);
        void updateAndApply(sf::Sprite& sprite, float dt);
    };
}