#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>

namespace game::components
{
    enum class StatusEffect { None, Poison };

    class Bullet
    {
    private:
        sf::Vector2f position;
        sf::Vector2f velocity;
        float speed = 800.0f;
        float speedMultiplier = 1.0f;
        bool isActive = true;

        bool useTexture = false;
        bool renderFallbackShape = true;
        sf::CircleShape shape;
        std::optional<sf::Sprite> sprite;
        std::shared_ptr<sf::Texture> texture;

        int currentFrame = 0;
        int totalFrames = 1;
        float frameTimer = 0.0f;
        float frameDuration = 0.1f;
        sf::Vector2i frameSize = { 0, 0 };

        float bulletRadius = 4.0f;
        sf::Color bulletColor = sf::Color(210, 180, 70);
        StatusEffect payloadStatus = StatusEffect::None;

        bool isWobbly = false;
        float lifetime = 0.0f;
        bool isFake3DRoll = false;
        float wobbleSpeed = 5.0f;
        float wobbleIntensity = 0.4f;
        float rollSpeed = 0.0f;
        float spinAmplitude = 45.0f;

        bool isParabolic = false;
        sf::Vector2f targetPos;
        sf::Vector2f startPos;
        float totalDistance = 0.0f;
        float maxArcHeight = 165.0f;
        float minArcHeight = 20.0f;

        sf::CircleShape shadowShape;
        bool spawnSplash = false;

    public:
        Bullet(sf::Vector2f startPos, sf::Vector2f direction);

        void update(float dt, const sf::Image& collisionMask, float mapScale);
        void render(sf::RenderWindow& window);

        void addVelocity(sf::Vector2f additionalVelocity);
        void destroy();
        void setupParabolic(sf::Vector2f start, sf::Vector2f target, float customSpeed);

        bool getIsActive() const;
        sf::Vector2f getPosition() const;
        float getRadius() const;
        StatusEffect getStatusEffect() const;

        void setAppearance(float radius, sf::Color color);
        void setSpeedMultiplier(float multi);
        void setStatusEffect(StatusEffect status);
        void setWobble(bool state, bool fake3DRoll = false);
        void setAnimation(std::shared_ptr<sf::Texture> tex, int frames, float animSpeed, sf::Vector2i size);

        bool consumeSplash();
    };
}