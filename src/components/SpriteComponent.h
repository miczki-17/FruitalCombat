#pragma once

#include "Component.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <optional>
#include "../utils/RoundedRectangleShape.h"

#include "PlayerInputComponent.h"

namespace game::components
{
    // Animation register
    enum class AnimState
    {
        Idle,
        Walk,
        Attack,
        Dead
    };

    struct AnimationData
    {
        sf::Texture* texture = nullptr;
        int totalFrames = 1;
        float frameDuration = 0.1f;
        bool loop = true;
    };

    class SpriteComponent final : public Component
    {
    public:
        SpriteComponent(
            sf::Texture* idleTexture,
            int idleFrames,
            sf::Texture* walkTexture,
            int walkFrames);

        SpriteComponent() = default;

        void update(float deltaTime) override;
        void render(sf::RenderWindow& window) override;

        void setTint(sf::Color color);
        void triggerHitFlash();
        void setCustomScale(float scaleModifier);

        // Pozwala fabrykom dodawac niestandardowe animacje
        void addAnimation(AnimState state, sf::Texture& texture, int frames, float duration, bool loop = true);

        void setTexture(std::shared_ptr<sf::Texture> tex);
        void setAlpha(uint8_t alpha);

    private:
        std::optional<sf::Sprite> sprite_;
        std::shared_ptr<sf::Texture> texture_;
        bool hasTextures_ = false;


        std::unordered_map<AnimState, AnimationData> animations_;
        AnimState currentState_ = AnimState::Idle;
        sf::Vector2i frameSize_{ 96, 96 };

        int currentFrame_ = 0;
        float animationTimer_ = 0.0f;
        bool animationFinished_ = false;

        sf::CircleShape fallbackShape_;

        // HP bar
        game::utils::RoundedRectangleShape hpBarBackground_;
        game::utils::RoundedRectangleShape hpBarFill_;

        // mana bar
        game::utils::RoundedRectangleShape manaBarBackground_;
        game::utils::RoundedRectangleShape manaBarFill_;

        sf::Color geneticColor_ = sf::Color::White;

        float hitFlashTimer_ = 0.0f;
        float currentScale_ = 1.0f;

        void initializeTextures(
            sf::Texture* idleTexture,
            int idleFrames,
            sf::Texture* walkTexture,
            int walkFrames);

        void initializeFallbackShape();
        void initializeHealthBar();

        void updateHitFlash(float deltaTime);
        void updateVisual(float deltaTime);
        void updateSprite(float deltaTime);
        void updateFallbackShape();
        void updateHealthBar();
        void restoreGeneticColor();

        bool hasSprite() const;
    };
}