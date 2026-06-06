// --- ProjectileComponent.cpp ---

#include "ProjectileComponent.h"
#include <cmath>
#include <algorithm>

namespace game::components
{
    ProjectileComponent::ProjectileComponent(sf::Vector2f startPos, sf::Vector2f direction)

        : position_(startPos)
    {

        float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (len != 0.0f) {
            direction.x /= len;
            direction.y /= len;
        }
        velocity_ = direction * speed_;



        shape_.setRadius(radius_);
        shape_.setOrigin({ radius_, radius_ });
        shape_.setFillColor(color_);
        shape_.setPosition(position_);
    }



    void ProjectileComponent::setupParabolic(sf::Vector2f start, sf::Vector2f target, float customSpeed)
    {
        isParabolic_ = true;
        startPos_ = start;
        targetPos_ = target;
        position_ = start;
        speed_ = customSpeed;



        sf::Vector2f diff = target - start;
        totalDistance_ = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (totalDistance_ > 0.0001f) {
            velocity_ = (diff / totalDistance_) * speed_;
        }



        shadowShape_.setRadius(radius_);
        shadowShape_.setFillColor(sf::Color(0, 0, 0, 150));
        shadowShape_.setOrigin({ radius_, radius_ });
    }



    void ProjectileComponent::setupDropFromSky(sf::Vector2f targetPos, float dropHeight, float customSpeed)
    {
        isDropping_ = true;
        targetPos_ = targetPos;
        startPos_ = { targetPos.x, targetPos.y - dropHeight };
        position_ = startPos_;
        speed_ = customSpeed;



        // velocity of scaling only in Y direction
        velocity_ = { 0.f, speed_ };
        totalDistance_ = dropHeight;


        shadowShape_.setRadius(radius_);
        shadowShape_.setFillColor(sf::Color(0, 0, 0, 150));
        shadowShape_.setOrigin({ radius_, radius_ });
    }



    void ProjectileComponent::addVelocity(sf::Vector2f additionalVelocity)
    {
        velocity_ += additionalVelocity;
    }



    void ProjectileComponent::destroy()
    {
        isActive_ = false;
    }



    bool ProjectileComponent::getIsActive() const { return isActive_; }
    sf::Vector2f ProjectileComponent::getPosition() const { return position_; }
    float ProjectileComponent::getRadius() const { return radius_; }
    StatusEffect ProjectileComponent::getStatusEffect() const { return payloadStatus_; }
    bool ProjectileComponent::consumeSplash()
    {
        if (spawnSplash_) {
            spawnSplash_ = false;
            return true;
        }
        return false;
    }



    void ProjectileComponent::setAppearance(float radius, sf::Color color)
    {
        radius_ = radius;
        color_ = color;
        shape_.setRadius(radius_);
        shape_.setOrigin({ radius_, radius_ });
        shape_.setFillColor(color_);
    }



    void ProjectileComponent::setSpeedMultiplier(float multi)
    {
        speedMultiplier_ = multi;
    }



    void ProjectileComponent::setStatusEffect(StatusEffect status)
    {
        payloadStatus_ = status;
    }



    void ProjectileComponent::setWobble(bool state, bool fake3DRoll)
    {
        isWobbly_ = state;
        isFake3DRoll_ = fake3DRoll;
    }



    void ProjectileComponent::setAnimation(std::shared_ptr<sf::Texture> tex, int frames, float animSpeed, sf::Vector2i size)
    {
        if (!tex) return;
        texture_ = tex;
        totalFrames_ = frames;
        frameDuration_ = animSpeed;
        frameSize_ = size;

        useTexture_ = true;
        renderFallbackShape_ = false;

        sprite_.emplace(*texture_);
        sprite_->setTextureRect(sf::IntRect({ 0, 0 }, frameSize_));
        sprite_->setOrigin({ frameSize_.x / 2.0f, frameSize_.y / 2.0f });

        if (!isWobbly_ && !isParabolic_ && !isDropping_) {
            float angle = std::atan2(velocity_.y, velocity_.x) * 180.0f / 3.14159f;

            // --- +90.0f ---
            // Ustawia "górê" obrazka zgodnie z wektorem lotu
            sprite_->setRotation(sf::degrees(angle + 90.0f));
        }
    }



    void ProjectileComponent::setSpriteScale(float scaleX, float scaleY)
    {
        if (sprite_.has_value()) {
            sprite_->setScale({ scaleX, scaleY });
        }
    }



    void ProjectileComponent::update(float dt, const sf::Image& collisionMask, float mapScale)
    {
        if (!isActive_) return;
        lifetime_ += dt;



        // --- Move from sky (vertical drop) ---

        if (isDropping_)
        {
            float distLeft = targetPos_.y - position_.y;
            if (distLeft <= 0.0f)
            {
                position_ = targetPos_;
                spawnSplash_ = true;
                destroy();
                return;
            }


            position_ += velocity_ * dt * speedMultiplier_;
            float progress = std::clamp(1.0f - (distLeft / totalDistance_), 0.0f, 1.0f);



            // shadow goes from 2.5x scale to 0 as it falls, and becomes more transparent
            float shadowScale = 2.5f * (1.0f - progress);

            shadowShape_.setPosition(targetPos_);
            shadowShape_.setScale({ shadowScale, shadowScale });
            shadowShape_.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(150 * (1.0f - progress))));

            shape_.setPosition(position_);
            if (sprite_.has_value()) sprite_->setPosition(position_);
        }

        // --- PARABOLIC MOVEMENT ---

        else if (isParabolic_)
        {
            sf::Vector2f diff = targetPos_ - position_;
            float currentDist = std::sqrt(diff.x * diff.x + diff.y * diff.y);


            if (currentDist < 15.0f)
            {
                position_ = targetPos_;
                spawnSplash_ = true;
                destroy();
                return;
            }

            position_ += velocity_ * dt * speedMultiplier_;


            float progress = 1.0f - (currentDist / totalDistance_);
            progress = std::clamp(progress, 0.0f, 1.0f);


            float currentArcHeight = maxArcHeight_ * std::sin(progress * 3.14159f);
            if (currentArcHeight < minArcHeight_) currentArcHeight = minArcHeight_;

            sf::Vector2f visualPos = position_;
            visualPos.y -= currentArcHeight;

            shadowShape_.setPosition(position_);

            float shadowScale = 1.0f - (currentArcHeight / maxArcHeight_) * 0.5f;
            shadowShape_.setScale({ shadowScale, shadowScale * 0.4f });

            shape_.setPosition(visualPos);
            if (sprite_.has_value()) sprite_->setPosition(visualPos);
        }

        // --- STANDARD MOVEMENT ON PLANE ---

        else
        {
            position_ += velocity_ * dt * speedMultiplier_;

            shape_.setPosition(position_);
            if (sprite_.has_value()) sprite_->setPosition(position_);



            // 1. check if projectile is out of bounds of the collision mask
            sf::Vector2i pixelPos(static_cast<int>(position_.x / mapScale), static_cast<int>(position_.y / mapScale));


            if (pixelPos.x < 0 || pixelPos.x >= static_cast<int>(collisionMask.getSize().x) ||
                pixelPos.y < 0 || pixelPos.y >= static_cast<int>(collisionMask.getSize().y))
            {
                destroy();
                return;
            }

            // 2. get pixel color from collision mask
            sf::Color pixel = collisionMask.getPixel({ static_cast<unsigned int>(pixelPos.x), static_cast<unsigned int>(pixelPos.y) });



            // if projectile hits a dark pixel (wall) -> destroy it
            if (pixel.r < 50 && pixel.g < 50 && pixel.b < 50)
            {
                destroy();
                return;
            }
        }

        // --- ANIMATION ---

        if (useTexture_ && sprite_.has_value())
        {
            frameTimer_ += dt;

            if (frameTimer_ >= frameDuration_) {
                frameTimer_ -= frameDuration_;
                currentFrame_ = (currentFrame_ + 1) % totalFrames_;
                sprite_->setTextureRect(sf::IntRect({ currentFrame_ * frameSize_.x, 0 }, frameSize_));
            }

            if (isWobbly_) {
                float wave = std::sin(lifetime_ * 15.0f) * 15.0f;
                sprite_->setRotation(sf::degrees(wave));
            }

            if (isFake3DRoll_) {
                sprite_->rotate(sf::degrees(dt * 360.0f));
            }
        }
    }

    void ProjectileComponent::update(float /*dt*/)
    {
        // empty 
    }

    void ProjectileComponent::render(sf::RenderWindow& window)
    {
        if (!isActive_) return;

        if (isParabolic_ || isDropping_) {
            window.draw(shadowShape_);
        }

        if (useTexture_ && sprite_.has_value()) {
            window.draw(*sprite_);
        }
        else if (renderFallbackShape_) {
            window.draw(shape_);
        }
    }

    void ProjectileComponent::enableShadow(float shadowRadius)
    {
        shadowShape_.setRadius(shadowRadius);
        shadowShape_.setOrigin({ shadowRadius, shadowRadius });
        shadowShape_.setFillColor(sf::Color(0, 0, 0, 150));
    }

    void ProjectileComponent::setImpactSound(const std::string& id)
    {
        impactSoundId_ = id;
    }

    const std::string& ProjectileComponent::getImpactSound() const
    {
        return impactSoundId_;
    }
}

