#include "Bullet.h"
#include <cmath>
#include <algorithm>

namespace game::components
{
    Bullet::Bullet(sf::Vector2f startPos, sf::Vector2f direction)
        : position(startPos)
    {
        shape.setRadius(bulletRadius);
        shape.setFillColor(bulletColor);
        shape.setOrigin({ bulletRadius, bulletRadius });
        shape.setPosition(position);

        velocity = direction * speed;
    }

    void Bullet::setAnimation(std::shared_ptr<sf::Texture> tex, int frames, float animSpeed, sf::Vector2i size)
    {
        if (!tex || tex->getSize().x == 0) return; // Zabezpieczenie przed pust? tekstur?

        useTexture = true;
        texture = tex;
        totalFrames = frames;
        frameDuration = animSpeed;
        frameSize = size;

        currentFrame = 0;
        frameTimer = frameDuration;

        sprite.emplace(*texture);
        sprite->setTextureRect(sf::IntRect({ 0, 0 }, frameSize));
        sprite->setOrigin({ static_cast<float>(frameSize.x) / 2.0f, static_cast<float>(frameSize.y) / 2.0f });
    }

    void Bullet::setupParabolic(sf::Vector2f start, sf::Vector2f target, float customSpeed)
    {
        isParabolic = true;
        startPos = start;
        position = start;
        targetPos = target;
        speed = customSpeed;

        sf::Vector2f toTarget = targetPos - startPos;
        totalDistance = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

        if (totalDistance > 0.001f) {
            velocity = (toTarget / totalDistance) * speed;
        }

        shadowShape.setRadius(bulletRadius);
        shadowShape.setFillColor(sf::Color(0, 0, 0, 100));
        shadowShape.setOrigin({ bulletRadius, bulletRadius });
    }

    void Bullet::addVelocity(sf::Vector2f additionalVelocity)
    {
        velocity += additionalVelocity;
    }

    void Bullet::update(float dt, const sf::Image& collisionMask, float mapScale)
    {
        if (!isActive) return;

        lifetime += dt;

        // --- FRAME ANIMATION LOGIC ---
        if (useTexture && totalFrames > 1 && sprite.has_value())
        {
            frameTimer -= dt;
            if (frameTimer <= 0.0f)
            {
                frameTimer = frameDuration;
                currentFrame = (currentFrame + 1) % totalFrames;
                sprite->setTextureRect(sf::IntRect({ currentFrame * frameSize.x, 0 }, frameSize));
            }
            sprite->setOrigin({ frameSize.x / 2.0f, frameSize.y / 2.0f });
        }

        if (isParabolic)
        {
            sf::Vector2f toTarget = targetPos - position;
            float distLeft = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

            if (distLeft < 8.0f || (velocity.x * toTarget.x + velocity.y * toTarget.y) < 0.0f)
            {
                position = targetPos;
                isActive = false;
                return;
            }

            position += velocity * dt;

            sf::Vector2f traveledVec = position - startPos;
            float traveledDist = std::sqrt(traveledVec.x * traveledVec.x + traveledVec.y * traveledVec.y);
            float progress = std::clamp(traveledDist / totalDistance, 0.0f, 1.0f);

            float currentHeight = 4.0f * arcHeight * progress * (1.0f - progress);

            // Pozycjonowanie
            sf::Vector2f visualPos = { position.x, position.y - currentHeight };
            shape.setPosition(visualPos);
            if (sprite.has_value()) sprite->setPosition(visualPos);

            // shadow
            shadowShape.setPosition(position);
            float shadowScale = 1.0f - (currentHeight / arcHeight) * 0.5f;
            shadowShape.setScale({ shadowScale, shadowScale });
            shadowShape.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(100 * (1.0f - progress * 0.3f))));

            // 3D Fake Roll
            if (isFake3DRoll)
            {
                float baseScaleX = 1.0f + wobbleIntensity * std::sin(lifetime * wobbleSpeed);
                float baseScaleY = 1.0f - wobbleIntensity * std::sin(lifetime * wobbleSpeed);
                float fakePitch = std::cos(lifetime * rollSpeed);

                sf::Vector2f finalScale = { baseScaleX, baseScaleY * fakePitch };
                shape.setScale(finalScale);
                if (sprite.has_value()) sprite->setScale(finalScale);

                auto finalRotation = sf::degrees(std::sin(lifetime * 8.0f) * spinAmplitude);
                shape.setRotation(finalRotation);
                if (sprite.has_value()) sprite->setRotation(finalRotation);
            }
            else if (isWobbly)
            {
                float scaleX = 1.0f + wobbleIntensity * std::sin(lifetime * wobbleSpeed);
                float scaleY = 1.0f - wobbleIntensity * std::sin(lifetime * wobbleSpeed);
                shape.setScale({ scaleX, scaleY });
                if (sprite.has_value()) sprite->setScale({ scaleX, scaleY });
            }

            float angleRad = std::atan2(velocity.y, velocity.x);
            float angleDeg = angleRad * 180.0f / 3.14159265f;

            if (sprite.has_value()) {
                sprite->setRotation(sf::degrees(angleDeg + 90.0f));
            }
        }
        else
        {
            if (isWobbly) {
                float scaleX = 1.0f + wobbleIntensity * std::sin(lifetime * wobbleSpeed);
                float scaleY = 1.0f - wobbleIntensity * std::sin(lifetime * wobbleSpeed);
                shape.setScale({ scaleX, scaleY });
                if (sprite.has_value()) sprite->setScale({ scaleX, scaleY });
            }
            position += velocity * speedMultiplier * dt;
            shape.setPosition(position);
            if (sprite.has_value()) sprite->setPosition(position);
        }

        // Kolizje
        sf::Vector2u maskSize = collisionMask.getSize();
        int px = static_cast<int>(position.x / mapScale);
        int py = static_cast<int>(position.y / mapScale);

        if (px < 0 || px >= static_cast<int>(maskSize.x) || py < 0 || py >= static_cast<int>(maskSize.y) ||
            collisionMask.getPixel(sf::Vector2u(px, py)) == sf::Color::Black)
        {
            isActive = false;
        }
    }

    void Bullet::render(sf::RenderWindow& window)
    {
        if (!isActive) return;

        if (isParabolic) {
            window.draw(shadowShape);
        }

        
        if (useTexture && sprite.has_value()) {
            window.draw(*sprite);
        }
        else {
            window.draw(shape);
        }
    }

    void Bullet::destroy() { isActive = false; }
    bool Bullet::getIsActive() const { return isActive; }
    sf::Vector2f Bullet::getPosition() const { return position; }
    float Bullet::getRadius() const { return shape.getRadius(); }
    StatusEffect Bullet::getStatusEffect() const { return payloadStatus; }

    void Bullet::setAppearance(float radius, sf::Color color)
    {
        bulletRadius = radius;
        bulletColor = color;
        shape.setRadius(bulletRadius);
        shape.setFillColor(bulletColor);
        shape.setOrigin({ bulletRadius, bulletRadius });
    }

    void Bullet::setSpeedMultiplier(float multi) { speedMultiplier = multi; }
    void Bullet::setStatusEffect(StatusEffect status) { payloadStatus = status; }
    void Bullet::setWobble(bool state, bool fake3DRoll) { isWobbly = state; isFake3DRoll = fake3DRoll; }
}