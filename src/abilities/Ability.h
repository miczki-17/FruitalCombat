// --- Ability.h ---

#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace game::components
{
    class Ability
    {
    protected:
        std::string sourceName_;

    public:
        explicit Ability(std::string sourceName = "Unidentified Biomass")
            : sourceName_(std::move(sourceName)) {}

        virtual ~Ability() = default;

        virtual void update(float deltaTime) = 0;

        virtual void execute(
            const sf::Vector2f& origin,
            const sf::Vector2f& targetPosition,
            const sf::Vector2f& ownerVelocity) = 0;

        void setSourceName(const std::string& newName) { sourceName_ = newName; }
        const std::string& getSourceName() const { return sourceName_; }
    };
}