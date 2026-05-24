#pragma once

#include "Component.h"
#include "../utils/AnimationController.h"
#include "StatsComponent.h"

#include <optional>

namespace game::components
{
    class SpriteComponent final : public Component
    {
    public:
        SpriteComponent(
            sf::Texture* idleTexture,
            int idleFrames,
            sf::Texture* walkTexture,
            int walkFrames);

        void update(float deltaTime) override;

        void render(
            sf::RenderWindow& window) override;

        void setTint(
            sf::Color color);

        void triggerHitFlash();

        void setCustomScale(
            float scaleModifier);

    private:
        std::optional<sf::Sprite> sprite_;
        AnimationController animator_;

        sf::CircleShape fallbackShape_;

        sf::RectangleShape hpBarBackground_;
        sf::RectangleShape hpBarFill_;

        sf::Color geneticColor_ =
            sf::Color::White;

        float hitFlashTimer_ =
            0.0f;

        bool hasTextures_ =
            false;

        float currentScale_ =
            1.0f;

        void initializeTextures(
            sf::Texture* idleTexture,
            int idleFrames,
            sf::Texture* walkTexture,
            int walkFrames);

        void initializeFallbackShape();

        void initializeHealthBar();

        void updateHitFlash(
            float deltaTime);

        void updateVisual();

        void updateSprite(
            float deltaTime);

        void updateFallbackShape();

        void updateHealthBar();

        void restoreGeneticColor();

        bool hasSprite() const;
    };
}