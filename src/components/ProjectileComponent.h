// --- ProjectileComponent.h ---

#pragma once
#include "Component.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <memory>

namespace game::components
{
    enum class StatusEffect 
    { 
        None,
        Poison,
        SporePoison,
        IceShatter
    };

    class ProjectileComponent final : public Component
    {
    public:
        ProjectileComponent(sf::Vector2f startPos, sf::Vector2f direction);

        void update(float dt, const sf::Image& collisionMask, float mapScale);
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;

        void setupParabolic(sf::Vector2f start, sf::Vector2f target, float customSpeed);
        void setupDropFromSky(sf::Vector2f targetPos, float dropHeight, float customSpeed);

        void addVelocity(sf::Vector2f additionalVelocity);
        void destroy();

        bool getIsActive() const;
        sf::Vector2f getPosition() const;
        float getRadius() const;
        StatusEffect getStatusEffect() const;
        float getDamage() const { return damage_; }
        void setDamage(float dmg) { damage_ = dmg; }

        void setAppearance(float radius, sf::Color color);
        void setSpeedMultiplier(float multi);
        void setStatusEffect(StatusEffect status);
        void setWobble(bool state, bool fake3DRoll = false);
        void setAnimation(std::shared_ptr<sf::Texture> tex, int frames, float animSpeed, sf::Vector2i size);
        void setSpriteScale(float scaleX, float scaleY);

        void setSplashKeyBase(const std::string& baseKey) { splashKeyBase_ = baseKey; }
        std::string getSplashKeyBase() const { return splashKeyBase_; }

        bool consumeSplash();

        bool getIsFriendly() const { return isFriendly_; }
        void setFriendly(bool friendly) { isFriendly_ = friendly; }


    private:
        sf::Vector2f position_;
        sf::Vector2f velocity_;
        float speed_ = 800.0f;
        float speedMultiplier_ = 1.0f;
        float lifetime_ = 0.0f;
        bool isActive_ = true;

        float damage_ = 25.0f; // default damage

        bool useTexture_ = false;
        bool renderFallbackShape_ = true;
        sf::CircleShape shape_;
        std::optional<sf::Sprite> sprite_;
        std::shared_ptr<sf::Texture> texture_;

        int currentFrame_ = 0;
        int totalFrames_ = 1;
        float frameTimer_ = 0.0f;
        float frameDuration_ = 0.1f;
        sf::Vector2i frameSize_{ 0, 0 };

        float radius_ = 4.0f;
        sf::Color color_ = sf::Color(210, 180, 70);

        StatusEffect payloadStatus_ = StatusEffect::None;

        bool isWobbly_ = false;
        bool isFake3DRoll_ = false;

        bool isParabolic_ = false;
        bool isDropping_ = false;

        sf::Vector2f targetPos_;
        sf::Vector2f startPos_;
        float totalDistance_ = 0.0f;
        float maxArcHeight_ = 120.0f;
        float minArcHeight_ = 20.0f;
        sf::CircleShape shadowShape_;
        bool spawnSplash_ = false;


        // default splashKey
        std::string splashKeyBase_ = "acid_splash";

        // bullets friendly logic
        bool isFriendly_ = true;
    };
}