// --- SpriteComponent.cpp ---

#include "SpriteComponent.h"
#include "../entities/Entity.h"

namespace game::components
{
    namespace
    {
        constexpr float BASE_SPRITE_SCALE =
            2.0f;

        constexpr float BASE_RADIUS =
            25.0f;

        constexpr float OUTLINE_THICKNESS =
            3.0f;

        constexpr float HIT_FLASH_DURATION =
            0.08f;

        constexpr float SPIN_SPEED =
            450.0f;

        constexpr float HEALTH_BAR_WIDTH =
            40.0f;

        constexpr float HEALTH_BAR_HEIGHT =
            6.0f;

        constexpr float HEALTH_BAR_OFFSET_Y =
            35.0f;
    }

    SpriteComponent::SpriteComponent(
        sf::Texture* idleTexture,
        int idleFrames,
        sf::Texture* walkTexture,
        int walkFrames)
    {
        initializeTextures(
            idleTexture,
            idleFrames,
            walkTexture,
            walkFrames);

        initializeFallbackShape();
        initializeHealthBar();
    }

    void SpriteComponent::update(
        float deltaTime)
    {
        updateHitFlash(
            deltaTime);

        updateVisual();

        updateHealthBar();
    }

    void SpriteComponent::render(
        sf::RenderWindow& window)
    {
        if (hasSprite())
        {
            window.draw(
                *sprite_);
        }
        else
        {
            window.draw(
                fallbackShape_);
        }

        auto* stats =
            owner->getComponent<
            StatsComponent>();

        if (
            owner &&
            !owner->isDead &&
            stats)
        {
            window.draw(
                hpBarBackground_);

            window.draw(
                hpBarFill_);
        }
    }

    void SpriteComponent::setTint(
        sf::Color color)
    {
        geneticColor_ =
            color;

        if (hasSprite())
        {
            sprite_->setColor(
                color);

            return;
        }

        fallbackShape_
            .setFillColor(color);
    }

    void SpriteComponent::triggerHitFlash()
    {
        hitFlashTimer_ =
            HIT_FLASH_DURATION;

        if (hasSprite())
        {
            sprite_->setColor(
                sf::Color::White);

            return;
        }

        fallbackShape_
            .setFillColor(
                sf::Color::White);
    }

    void SpriteComponent::setCustomScale(
        float scaleModifier)
    {
        currentScale_ =
            scaleModifier;

        if (hasSprite())
        {
            sprite_->setScale({
                BASE_SPRITE_SCALE *
                    scaleModifier,
                BASE_SPRITE_SCALE *
                    scaleModifier
                });
        }

        fallbackShape_
            .setRadius(
                BASE_RADIUS *
                scaleModifier);

        fallbackShape_
            .setOrigin({
                BASE_RADIUS *
                    scaleModifier,
                BASE_RADIUS *
                    scaleModifier
                });
    }

    void SpriteComponent::initializeTextures(
        sf::Texture* idleTexture,
        int idleFrames,
        sf::Texture* walkTexture,
        int walkFrames)
    {
        const bool validTextures =
            idleTexture &&
            walkTexture &&
            idleTexture
            ->getSize()
            .x > 0 &&
            walkTexture
            ->getSize()
            .x > 0;

        if (!validTextures)
        {
            return;
        }

        animator_.setTextures(
            *idleTexture,
            idleFrames,
            *walkTexture,
            walkFrames);

        sprite_.emplace(
            animator_
            .getDefaultTexture());

        sprite_->setOrigin({
            32.f,
            32.f
            });

        sprite_->setScale({
            BASE_SPRITE_SCALE,
            BASE_SPRITE_SCALE
            });

        hasTextures_ = true;
    }

    void SpriteComponent::initializeFallbackShape()
    {
        fallbackShape_
            .setRadius(
                BASE_RADIUS);

        fallbackShape_
            .setOrigin({
                BASE_RADIUS,
                BASE_RADIUS
                });

        fallbackShape_
            .setOutlineThickness(
                OUTLINE_THICKNESS);

        fallbackShape_
            .setOutlineColor(
                sf::Color(
                    0,
                    0,
                    0,
                    150));
    }

    void SpriteComponent::initializeHealthBar()
    {
        hpBarBackground_
            .setFillColor(
                sf::Color(
                    50,
                    50,
                    50,
                    200));

        hpBarFill_
            .setFillColor(
                sf::Color(
                    255,
                    50,
                    50,
                    255));
    }

    void SpriteComponent::updateHitFlash(
        float deltaTime)
    {
        if (
            hitFlashTimer_ <=
            0.0f)
        {
            return;
        }

        hitFlashTimer_ -=
            deltaTime;

        if (
            hitFlashTimer_ <=
            0.0f)
        {
            restoreGeneticColor();
        }
    }

    void SpriteComponent::updateVisual()
    {
        if (hasSprite())
        {
            updateSprite(
                0.0f);

            return;
        }

        updateFallbackShape();
    }

    void SpriteComponent::updateSprite(
        float deltaTime)
    {
        sprite_->setPosition(
            owner->position);

        if (
            owner->actionTimer >
            0.0f)
        {
            sprite_->rotate(
                sf::degrees(
                    SPIN_SPEED *
                    deltaTime));

            return;
        }

        sprite_->setRotation(
            sf::degrees(
                0.f));

        animator_
            .setMovementState(
                owner->isMoving,
                owner->facingRight);

        animator_
            .updateAndApply(
                *sprite_,
                deltaTime);
    }

    void SpriteComponent::updateFallbackShape()
    {
        fallbackShape_
            .setPosition(
                owner->position);
    }

    void SpriteComponent::updateHealthBar()
    {
        auto* stats =
            owner->getComponent<
            StatsComponent>();

        if (!stats)
        {
            return;
        }

        const float width =
            HEALTH_BAR_WIDTH *
            currentScale_;

        hpBarBackground_
            .setSize({
                width,
                HEALTH_BAR_HEIGHT
                });

        hpBarBackground_
            .setOrigin({
                width / 2.0f,
                HEALTH_BAR_HEIGHT /
                    2.0f
                });

        hpBarBackground_
            .setPosition({
                owner->position.x,
                owner->position.y -
                    (
                        HEALTH_BAR_OFFSET_Y *
                        currentScale_
                    )
                });

        hpBarFill_
            .setSize({
                width *
                    stats
                        ->getHpPercentage(),
                HEALTH_BAR_HEIGHT
                });

        hpBarFill_
            .setOrigin({
                width / 2.0f,
                HEALTH_BAR_HEIGHT /
                    2.0f
                });

        hpBarFill_
            .setPosition({
                owner->position.x,
                owner->position.y -
                    (
                        HEALTH_BAR_OFFSET_Y *
                        currentScale_
                    )
                });
    }

    void SpriteComponent::restoreGeneticColor()
    {
        if (hasSprite())
        {
            sprite_->setColor(
                geneticColor_);

            return;
        }

        fallbackShape_
            .setFillColor(
                geneticColor_);
    }

    bool SpriteComponent::hasSprite() const
    {
        return
            hasTextures_ &&
            sprite_.has_value();
    }
}