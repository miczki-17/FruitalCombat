#include "SpriteComponent.h"
#include "StatsComponent.h"
#include "TransformComponent.h"
#include "../entities/Entity.h"
#include "../core/Game.h"

namespace game::components
{
    namespace
    {
        constexpr float BASE_SPRITE_SCALE = 2.0f;
        constexpr float BASE_RADIUS = 25.0f;
        constexpr float OUTLINE_THICKNESS = 3.0f;
        constexpr float HIT_FLASH_DURATION = 0.08f;
        constexpr float SPIN_SPEED = 450.0f;

        constexpr float HEALTH_BAR_WIDTH = 40.0f;
        constexpr float HEALTH_BAR_HEIGHT = 6.0f;
        constexpr float HEALTH_BAR_OFFSET_Y = 35.0f;
    }

    SpriteComponent::SpriteComponent(
        sf::Texture* idleTexture,
        int idleFrames,
        sf::Texture* walkTexture,
        int walkFrames)
    {
        initializeTextures(idleTexture, idleFrames, walkTexture, walkFrames);
        initializeFallbackShape();
        initializeHealthBar();
    }

    void SpriteComponent::addAnimation(AnimState state, sf::Texture& texture, int frames, float duration, bool loop)
    {
        animations_[state] = { &texture, frames, duration, loop };
    }

    void SpriteComponent::update(float deltaTime)
    {
        updateHitFlash(deltaTime);
        updateVisual(deltaTime);
        updateHealthBar();

        if (hitFlashTimer_ <= 0.0f)
        {
            auto* stats = owner->getComponent<StatsComponent>();
            if (stats)
            {
                // zaczynamy od default coloru
                sf::Color targetColor = geneticColor_;

                // Sprawdzamy efekty
                if (stats->hasActiveEffect(StatusType::Slow))
                {
                    // Lekko ¿ó³tawo-zielony (jak cydr/kwas)
                    targetColor = sf::Color(170, 255, 120, geneticColor_.a);
                }
                else if (stats->hasActiveEffect(StatusType::Poison))
                {
                    // G³êboki fioletowy/zielony dla zwyk³ej trucizny
                    targetColor = sf::Color(200, 100, 255, geneticColor_.a);
                }

                // Aplikujemy ostateczny kolor
                if (hasSprite()) {
                    sprite_->setColor(targetColor);
                }
                else {
                    fallbackShape_.setFillColor(targetColor);
                }
            }
        }
    }

    void SpriteComponent::render(sf::RenderWindow& window)
    {
        if (hasSprite() && sprite_.has_value())
        {
            auto* transform = owner->getComponent<TransformComponent>();
            if (transform) {
                sprite_->setPosition(transform->position);
                sprite_->setRotation(sf::degrees(transform->rotation));
                sprite_->setScale(transform->scale);
            }
            window.draw(*sprite_);
        }

        if (hasSprite())
            window.draw(*sprite_);
        else
            window.draw(fallbackShape_);

        auto* stats = owner->getComponent<StatsComponent>();
        auto* transform = owner->getComponent<TransformComponent>();
        if (transform && !owner->isDead() && stats)
        {
            // Rysujemy HP dla wszystkich
            window.draw(hpBarBackground_);
            window.draw(hpBarFill_);

            // Rysujemy Mane tylko dla gracza
            if (owner->getComponent<PlayerInputComponent>() != nullptr) {
                window.draw(manaBarBackground_);
                window.draw(manaBarFill_);
            }
        }
    }

    void SpriteComponent::updateVisual(float deltaTime)
    {
        if (hasSprite())
            updateSprite(deltaTime);
        else
            updateFallbackShape();
    }

    void SpriteComponent::updateSprite(float deltaTime)
    {
        auto* transform = owner->getComponent<TransformComponent>();
        sprite_->setPosition(transform->position);

        // 1. Obsluga rotacji (zdolnoœci specjalne typu RindRoll)
        if (transform->actionTimer > 0.0f)
        {
            sprite_->rotate(sf::degrees(SPIN_SPEED * deltaTime));
        }
        else
        {
            sprite_->setRotation(sf::degrees(0.f));
        }

        // 2. Wybor odpowiedniego stanu na podstawie flag Entity
        AnimState requiredState = AnimState::Idle;
        if (owner->isDead())         requiredState = AnimState::Dead;
        else if (owner->isAttacking()) requiredState = AnimState::Attack;
        else if (transform->isMoving)    requiredState = AnimState::Walk;

        // Jesli stan sie zmienil, resetujemy licznik klatek
        if (currentState_ != requiredState)
        {
            currentState_ = requiredState;
            currentFrame_ = 0;
            animationTimer_ = 0.0f;
            animationFinished_ = false;
        }

        // 3. Logika wewnetrznej aktualizacji animacji
        auto it = animations_.find(currentState_);
        if (it != animations_.end() && it->second.texture)
        {
            const AnimationData& data = it->second;
            sprite_->setTexture(*(data.texture), true);

            if (!animationFinished_)
            {
                animationTimer_ += deltaTime;
                if (animationTimer_ >= data.frameDuration)
                {
                    animationTimer_ -= data.frameDuration;
                    currentFrame_++;

                    if (currentFrame_ >= data.totalFrames)
                    {
                        if (data.loop)
                        {
                            currentFrame_ = 0;
                        }
                        else
                        {
                            currentFrame_ = data.totalFrames - 1;
                            animationFinished_ = true;
                        }
                    }
                }
            }

            // 4. Obliczanie wycinka tekstury i obsluga odbicia lustrzanego (facingRight)
            int rectLeftX = currentFrame_ * frameSize_.x;

            if (transform->facingRight)
            {
                sprite_->setTextureRect(sf::IntRect({ rectLeftX, 0 }, frameSize_));
            }
            else
            {
                sprite_->setTextureRect(sf::IntRect({ rectLeftX + frameSize_.x, 0 }, { -frameSize_.x, frameSize_.y }));
            }
        }
    }

    void SpriteComponent::updateFallbackShape()
    {
        auto* transform = owner->getComponent<TransformComponent>();
        fallbackShape_.setPosition(transform->position);
    }

    void SpriteComponent::updateHitFlash(float deltaTime)
    {
        if (hitFlashTimer_ <= 0.0f)
            return;

        hitFlashTimer_ -= deltaTime;

        if (hitFlashTimer_ <= 0.0f)
            restoreGeneticColor();
    }

    void SpriteComponent::setTint(sf::Color color)
    {
        geneticColor_ = color;

        if (hasSprite())
            sprite_->setColor(color);
        else
            fallbackShape_.setFillColor(color);
    }

    void SpriteComponent::triggerHitFlash()
    {
        hitFlashTimer_ = HIT_FLASH_DURATION;

        if (hasSprite())
            sprite_->setColor(sf::Color::White);
        else
            fallbackShape_.setFillColor(sf::Color::White);
    }

    void SpriteComponent::setCustomScale(float scaleModifier)
    {
        currentScale_ = scaleModifier;

        if (hasSprite())
        {
            sprite_->setScale({
                BASE_SPRITE_SCALE * scaleModifier,
                BASE_SPRITE_SCALE * scaleModifier
                });
        }

        fallbackShape_.setRadius(BASE_RADIUS * scaleModifier);
        fallbackShape_.setOrigin({
            BASE_RADIUS * scaleModifier,
            BASE_RADIUS * scaleModifier
            });
    }

    void SpriteComponent::initializeTextures(
        sf::Texture* idleTexture,
        int idleFrames,
        sf::Texture* walkTexture,
        int walkFrames)
    {
        const bool valid =
            idleTexture &&
            walkTexture &&
            idleTexture->getSize().x > 0 &&
            walkTexture->getSize().x > 0;

        if (!valid)
            return;

        // Wstrzykujemy animacje bezposrednio do mapy komponentu
        addAnimation(AnimState::Idle, *idleTexture, idleFrames, 0.15f, true);
        addAnimation(AnimState::Walk, *walkTexture, walkFrames, 0.10f, true);

        sprite_.emplace(*idleTexture);
        sprite_->setOrigin({ 32.f, 32.f });
        sprite_->setScale({ BASE_SPRITE_SCALE, BASE_SPRITE_SCALE });

        hasTextures_ = true;
    }

    void SpriteComponent::initializeFallbackShape()
    {
        fallbackShape_.setRadius(BASE_RADIUS);
        fallbackShape_.setOrigin({ BASE_RADIUS, BASE_RADIUS });
        fallbackShape_.setOutlineThickness(OUTLINE_THICKNESS);
        fallbackShape_.setOutlineColor(sf::Color(0, 0, 0, 150));
    }

    void SpriteComponent::initializeHealthBar()
    {
        hpBarBackground_.setRadius(HEALTH_BAR_HEIGHT / 2.0f);
        hpBarBackground_.setFillColor(sf::Color(50, 50, 50, 200));
        hpBarFill_.setRadius(HEALTH_BAR_HEIGHT / 2.0f);
        hpBarFill_.setFillColor(sf::Color(255, 50, 50, 255));

        manaBarBackground_.setRadius(HEALTH_BAR_HEIGHT / 2.0f);
        manaBarBackground_.setFillColor(sf::Color(50, 50, 50, 200));
        manaBarFill_.setRadius(HEALTH_BAR_HEIGHT / 2.0f);
        manaBarFill_.setFillColor(sf::Color(255, 215, 0, 255));
    }

    void SpriteComponent::updateHealthBar()
    {
        auto* stats = owner->getComponent<StatsComponent>();
        auto* transform = owner->getComponent<TransformComponent>();
        if (!stats || !transform) return;

        const float width = HEALTH_BAR_WIDTH * currentScale_;

        // --- Pasek HP (Rysowany dla kazdego: gracza i wrogow) ---
        hpBarBackground_.setSize({ width, HEALTH_BAR_HEIGHT });
        hpBarBackground_.setOrigin({ width / 2.f, HEALTH_BAR_HEIGHT / 2.f });
        hpBarBackground_.setPosition({
            transform->position.x,
            transform->position.y - HEALTH_BAR_OFFSET_Y * currentScale_
            });

        float hpFillWidth = std::max(width * stats->getHpPercentage(), HEALTH_BAR_HEIGHT);
        hpBarFill_.setSize({ hpFillWidth, HEALTH_BAR_HEIGHT });
        hpBarFill_.setOrigin({ width / 2.f, HEALTH_BAR_HEIGHT / 2.f });
        hpBarFill_.setPosition({
            transform->position.x,
            transform->position.y - HEALTH_BAR_OFFSET_Y * currentScale_
            });

        // --- Pasek Many (Rysowany TYLKO dla gracza) ---
        bool isPlayer = (owner->getComponent<PlayerInputComponent>() != nullptr);
        if (isPlayer) {
            // second bar offeset
            float manaYOffset = (HEALTH_BAR_OFFSET_Y - 8.5f) * currentScale_;

            manaBarBackground_.setSize({ width, HEALTH_BAR_HEIGHT });
            manaBarBackground_.setOrigin({ width / 2.f, HEALTH_BAR_HEIGHT / 2.f });
            manaBarBackground_.setPosition({ transform->position.x, transform->position.y - manaYOffset });

            float manaFillWidth = std::max(width * stats->getManaPercentage(), HEALTH_BAR_HEIGHT);
            manaBarFill_.setSize({ manaFillWidth, HEALTH_BAR_HEIGHT });
            manaBarFill_.setOrigin({ width / 2.f, HEALTH_BAR_HEIGHT / 2.f });
            manaBarFill_.setPosition({ transform->position.x, transform->position.y - manaYOffset });
        }
    }

    void SpriteComponent::restoreGeneticColor()
    {
        if (hasSprite())
            sprite_->setColor(geneticColor_);
        else
            fallbackShape_.setFillColor(geneticColor_);
    }

    bool SpriteComponent::hasSprite() const
    {
        return hasTextures_;
    }


    void SpriteComponent::setAlpha(uint8_t alpha) {
        if (sprite_.has_value())
        {
            sf::Color c = sprite_->getColor();
            c.a = alpha;
            sprite_->setColor(c);
        }
    }

    void SpriteComponent::setTexture(std::shared_ptr<sf::Texture> tex) {
        texture_ = tex;
        sprite_.emplace(*texture_);

        auto size = texture_->getSize();
        sprite_->setOrigin({ size.x / 2.0f, size.y / 2.0f });

        hasTextures_ = true;
    }
}