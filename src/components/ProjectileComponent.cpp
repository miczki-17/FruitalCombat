// ==========================================
// components/ProjectileComponent.cpp
// ==========================================
#include "ProjectileComponent.h"
#include <cmath>
#include <algorithm>

namespace game::components
{
    ProjectileComponent::ProjectileComponent(sf::Vector2f startPos, sf::Vector2f direction)
        : position_(startPos)
    {
        shape_.setRadius(radius_);
        shape_.setFillColor(color_);
        shape_.setOrigin({ radius_, radius_ });
        shape_.setPosition(position_);

        velocity_ = direction * speed_;
    }

    void ProjectileComponent::setAppearance(float radius, sf::Color color)
    {
        radius_ = radius;
        color_ = color;
        shape_.setRadius(radius_);
        shape_.setFillColor(color_);
        shape_.setOrigin({ radius_, radius_ });
    }

    void ProjectileComponent::setAnimation(std::shared_ptr<sf::Texture> tex, int frames, float animSpeed, sf::Vector2i size)
    {
        if (!tex || tex->getSize().x == 0 || tex->getSize().y == 0) return;

        useTexture_ = true;
        renderFallbackShape_ = false;
        texture_ = tex;
        totalFrames_ = frames;
        frameDuration_ = animSpeed;
        frameSize_ = size;
        currentFrame_ = 0;
        frameTimer_ = frameDuration_;

        sprite_.emplace(*texture_);
        sprite_->setTextureRect(sf::IntRect({ 0, 0 }, frameSize_));
        sprite_->setOrigin({ frameSize_.x / 2.0f, frameSize_.y / 2.0f });
    }

    void ProjectileComponent::setupParabolic(sf::Vector2f start, sf::Vector2f target, float customSpeed)
    {
        isParabolic_ = true;
        startPos_ = start;
        position_ = start;
        targetPos_ = target;
        speed_ = customSpeed;

        sf::Vector2f toTarget = targetPos_ - startPos_;
        totalDistance_ = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

        if (totalDistance_ > 0.001f)
            velocity_ = (toTarget / totalDistance_) * speed_;

        shadowShape_.setRadius(radius_);
        shadowShape_.setFillColor(sf::Color(0, 0, 0, 100));
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
    void ProjectileComponent::setSpeedMultiplier(float multi) { speedMultiplier_ = multi; }
    void ProjectileComponent::setStatusEffect(StatusEffect status) { payloadStatus_ = status; }

    void ProjectileComponent::update(float dt)
    {
        // Pusta implementacja dla Component::update
    }

    void ProjectileComponent::update(float dt, const sf::Image& collisionMask, float mapScale)
    {
        if (!isActive_) return;

        lifetime_ += dt;
        if (lifetime_ > 8.0f)
        {
            destroy();
            return;
        }

        if (mapScale <= 0.0001f) return;

        // --- ANIMACJA ---
        if (useTexture_ && totalFrames_ > 1 && sprite_.has_value())
        {
            frameTimer_ -= dt;
            if (frameTimer_ <= 0.0f)
            {
                frameTimer_ = frameDuration_;
                currentFrame_ = (currentFrame_ + 1) % totalFrames_;
                sprite_->setTextureRect(sf::IntRect({ currentFrame_ * frameSize_.x, 0 }, frameSize_));
            }
        }

        // --- RUCH ---
        if (isParabolic_)
        {
            sf::Vector2f toTarget = targetPos_ - position_;
            float distLeft = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

            if (distLeft < 8.0f || (velocity_.x * toTarget.x + velocity_.y * toTarget.y) < 0.0f)
            {
                position_ = targetPos_;
                spawnSplash_ = true;
                destroy();
                return;
            }

            position_ += velocity_ * dt;

            sf::Vector2f traveledVec = position_ - startPos_;
            float traveledDist = std::sqrt(traveledVec.x * traveledVec.x + traveledVec.y * traveledVec.y);
            float progress = std::clamp(traveledDist / totalDistance_, 0.0f, 1.0f);

            float distanceFactor = std::clamp(totalDistance_ / 900.0f, 0.0f, 1.0f);
            float arcHeight = minArcHeight_ + distanceFactor * (maxArcHeight_ - minArcHeight_);
            float currentHeight = 4.0f * arcHeight * progress * (1.0f - progress);

            sf::Vector2f visualPos = { position_.x, position_.y - currentHeight };
            shape_.setPosition(visualPos);
            if (sprite_.has_value()) sprite_->setPosition(visualPos);

            shadowShape_.setPosition(position_);
            float shadowScale = 1.0f - (currentHeight / arcHeight) * 0.5f;
            shadowShape_.setScale({ shadowScale, shadowScale });
            shadowShape_.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(100 * (1.0f - progress * 0.3f))));

            float angleDeg = std::atan2(velocity_.y, velocity_.x) * 180.0f / 3.14159265f;
            if (sprite_.has_value()) sprite_->setRotation(sf::degrees(angleDeg + 90.0f));
        }
        else
        {
            position_ += velocity_ * speedMultiplier_ * dt;
            shape_.setPosition(position_);
            if (sprite_.has_value()) sprite_->setPosition(position_);
        }

        // --- KOLIZJA ---
        sf::Vector2u maskSize = collisionMask.getSize();
        int px = static_cast<int>(position_.x / mapScale);
        int py = static_cast<int>(position_.y / mapScale);

        if (px < 0 || py < 0 || px >= static_cast<int>(maskSize.x) || py >= static_cast<int>(maskSize.y) ||
            collisionMask.getPixel(sf::Vector2u(px, py)) == sf::Color::Black)
        {
            destroy();
        }
    }

    void ProjectileComponent::render(sf::RenderWindow& window)
    {
        if (!isActive_) return;

        if (isParabolic_) window.draw(shadowShape_);
        if (useTexture_ && sprite_.has_value()) window.draw(*sprite_);
        else if (renderFallbackShape_) window.draw(shape_);
    }

    bool ProjectileComponent::consumeSplash()
    {
        bool val = spawnSplash_;
        spawnSplash_ = false;
        return val;
    }
}