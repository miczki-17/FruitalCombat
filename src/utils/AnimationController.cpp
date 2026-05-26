// --- AnimationController.cpp ---

#include "AnimationController.h"
#include <iostream>

namespace game::components
{
    void AnimationController::setTextures(sf::Texture& idle, int idleFrames, sf::Texture& walk, int walkFrames)
    {
        idleTexture = &idle;
        walkTexture = &walk;
        totalIdleFrames = idleFrames;
        totalWalkFrames = walkFrames;
    }

    const sf::Texture& AnimationController::getDefaultTexture() const
    {
        return *idleTexture;
    }

    void AnimationController::setMovementState(bool moving, bool right)
    {
        isMoving = moving;
        facingRight = right;
    }

    void AnimationController::updateAndApply(sf::Sprite& sprite, float dt)
    {
        if (!idleTexture || !walkTexture) return;

        if (isMoving) sprite.setTexture(*walkTexture, true);
        else          sprite.setTexture(*idleTexture, true);

        int maxFrames = isMoving ? totalWalkFrames : totalIdleFrames;
        float frameDuration = isMoving ? 0.1f : 0.15f;

        if (currentFrame >= maxFrames) currentFrame = 0;

        animationTimer += dt;
        if (animationTimer >= frameDuration)
        {
            animationTimer -= frameDuration;
            currentFrame++;
            if (currentFrame >= maxFrames) currentFrame = 0;
        }

        int rectLeftX = currentFrame * frameSize.x;

        if (facingRight)
        {
            sprite.setTextureRect(sf::IntRect({ rectLeftX, 0 }, frameSize));
        }
        else
        {
            sprite.setTextureRect(sf::IntRect({ rectLeftX + frameSize.x, 0 }, { -frameSize.x, frameSize.y }));
        }
    }
}