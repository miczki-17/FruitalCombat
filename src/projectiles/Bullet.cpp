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

    void Bullet::setAnimation(std::shared_ptr<sf::Texture> tex,
        int frames,
        float animSpeed,
        sf::Vector2i size)
    {
        if (!tex || tex->getSize().x == 0 || tex->getSize().y == 0)
            return;

        useTexture = true;
        renderFallbackShape = false;

        texture = tex;
        totalFrames = frames;
        frameDuration = animSpeed;
        frameSize = size;

        currentFrame = 0;
        frameTimer = frameDuration;

        sprite.emplace(*texture);

        sprite->setTextureRect(sf::IntRect({ 0, 0 }, frameSize));
        sprite->setOrigin({
            frameSize.x / 2.0f,
            frameSize.y / 2.0f
            });
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

        if (totalDistance > 0.001f)
            velocity = (toTarget / totalDistance) * speed;

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

        if (lifetime > 8.0f)
        {
            isActive = false;
            return;
        }

        if (mapScale <= 0.0001f)
            return;

        // --- ANIMATION ---
        if (useTexture && totalFrames > 1 && sprite.has_value())
        {
            frameTimer -= dt;

            if (frameTimer <= 0.0f)
            {
                frameTimer = frameDuration;
                currentFrame = (currentFrame + 1) % totalFrames;

                sprite->setTextureRect(sf::IntRect(
                    { currentFrame * frameSize.x, 0 },
                    frameSize
                ));
            }
        }

        // --- PARABOLIC MOVEMENT ---
        if (isParabolic)
        {
            sf::Vector2f toTarget = targetPos - position;
            float distLeft = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

            if (distLeft < 8.0f ||
                (velocity.x * toTarget.x + velocity.y * toTarget.y) < 0.0f)
            {
                position = targetPos;
                spawnSplash = true;
                isActive = false;
                return;
            }

            position += velocity * dt;

            sf::Vector2f traveledVec = position - startPos;
            float traveledDist = std::sqrt(traveledVec.x * traveledVec.x + traveledVec.y * traveledVec.y);

            float progress = std::clamp(traveledDist / totalDistance, 0.0f, 1.0f);

            float maxDistanceForArc = 900.0f;
            float distanceFactor = std::clamp(totalDistance / maxDistanceForArc, 0.0f, 1.0f);

            float arcHeight = minArcHeight +
                distanceFactor * (maxArcHeight - minArcHeight);

            float currentHeight = 4.0f * arcHeight * progress * (1.0f - progress);

            sf::Vector2f visualPos = { position.x, position.y - currentHeight };

            shape.setPosition(visualPos);
            if (sprite.has_value()) sprite->setPosition(visualPos);

            shadowShape.setPosition(position);

            float shadowScale = 1.0f - (currentHeight / arcHeight) * 0.5f;
            shadowShape.setScale({ shadowScale, shadowScale });

            shadowShape.setFillColor(sf::Color(
                0, 0, 0,
                static_cast<uint8_t>(100 * (1.0f - progress * 0.3f))
            ));

            float angleRad = std::atan2(velocity.y, velocity.x);
            float angleDeg = angleRad * 180.0f / 3.14159265f;

            if (sprite.has_value())
                sprite->setRotation(sf::degrees(angleDeg + 90.0f));
        }
        else
        {
            position += velocity * speedMultiplier * dt;

            shape.setPosition(position);
            if (sprite.has_value())
                sprite->setPosition(position);
        }

        // --- COLLISION ---
        sf::Vector2u maskSize = collisionMask.getSize();

        int px = static_cast<int>(position.x / mapScale);
        int py = static_cast<int>(position.y / mapScale);

        if (px < 0 || py < 0 ||
            px >= static_cast<int>(maskSize.x) ||
            py >= static_cast<int>(maskSize.y) ||
            collisionMask.getPixel(sf::Vector2u(px, py)) == sf::Color::Black)
        {
            isActive = false;
        }
    }

    void Bullet::render(sf::RenderWindow& window)
    {
        if (!isActive) return;

        if (isParabolic)
            window.draw(shadowShape);

        if (useTexture && sprite.has_value())
            window.draw(*sprite);
        else if (renderFallbackShape)
            window.draw(shape);
    }

    void Bullet::destroy()
    {
        isActive = false;
    }

    bool Bullet::getIsActive() const
    {
        return isActive;
    }

    sf::Vector2f Bullet::getPosition() const
    {
        return position;
    }

    float Bullet::getRadius() const
    {
        return shape.getRadius();
    }

    StatusEffect Bullet::getStatusEffect() const
    {
        return payloadStatus;
    }

    void Bullet::setAppearance(float radius, sf::Color color)
    {
        bulletRadius = radius;
        bulletColor = color;

        shape.setRadius(bulletRadius);
        shape.setFillColor(bulletColor);
        shape.setOrigin({ bulletRadius, bulletRadius });
    }

    void Bullet::setSpeedMultiplier(float multi)
    {
        speedMultiplier = multi;
    }

    void Bullet::setStatusEffect(StatusEffect status)
    {
        payloadStatus = status;
    }

    void Bullet::setWobble(bool state, bool fake3DRoll)
    {
        isWobbly = state;
        isFake3DRoll = fake3DRoll;
    }

    bool Bullet::consumeSplash()
    {
        bool value = spawnSplash;
        spawnSplash = false;
        return value;
    }
}