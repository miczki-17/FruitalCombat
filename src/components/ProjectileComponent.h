// ==========================================
// components/ProjectileComponent.h
// ==========================================
#pragma once
#include "Component.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <memory>

namespace game::components
{
    enum class StatusEffect { None, Poison };

    class ProjectileComponent final : public Component
    {
    public:
        // Przywrócony stary konstruktor!
        ProjectileComponent(sf::Vector2f startPos, sf::Vector2f direction);

        // Tymczasowa metoda dla kompatybilno?ci z PlayingState
        void update(float dt, const sf::Image& collisionMask, float mapScale);
        void update(float dt) override; // Wymagane przez Component bazowy
        void render(sf::RenderWindow& window) override;

        void setupParabolic(sf::Vector2f start, sf::Vector2f target, float customSpeed);
        void addVelocity(sf::Vector2f additionalVelocity);
        void destroy();

        // Przywrócone stare nazwy getterów!
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

    private:
        sf::Vector2f position_; // Tymczasowo w?asna pozycja, bo obiekt le?y luzem w wektorze
        sf::Vector2f velocity_;
        float speed_ = 800.0f;
        float speedMultiplier_ = 1.0f;
        float lifetime_ = 0.0f;
        bool isActive_ = true;

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
        sf::Vector2f targetPos_;
        sf::Vector2f startPos_;
        float totalDistance_ = 0.0f;
        float maxArcHeight_ = 165.0f;
        float minArcHeight_ = 20.0f;
        sf::CircleShape shadowShape_;
        bool spawnSplash_ = false;
    };
}