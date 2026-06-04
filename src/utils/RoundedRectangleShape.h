// --- RoundedRectangleShape.h ---

#pragma once
#include <SFML/Graphics/Shape.hpp>
#include <cmath>
#include <algorithm>

namespace game::utils
{
    class RoundedRectangleShape : public sf::Shape
    {
    public:
        explicit RoundedRectangleShape(const sf::Vector2f& size = { 0.f, 0.f }, float radius = 0.f, std::size_t cornerPointCount = 8)
        {
            size_ = size;
            radius_ = radius;
            cornerPointCount_ = cornerPointCount;
            update();
        }

        void setSize(const sf::Vector2f& size) { size_ = size; update(); }
        void setRadius(float radius) { radius_ = radius; update(); }

        std::size_t getPointCount() const override { return cornerPointCount_ * 4; }

        sf::Vector2f getPoint(std::size_t index) const override
        {
            float deltaAngle = 90.0f / (cornerPointCount_ - 1);
            sf::Vector2f center;
            std::size_t centerIndex = index / cornerPointCount_;

            // Zabezpieczenie: promien nie moze byc wiekszy niz polowa szerokosci/wysokosci
            float r = std::min({ radius_, size_.x / 2.0f, size_.y / 2.0f });

            switch (centerIndex) {
            case 0: center.x = size_.x - r; center.y = r; break;             // Prawy gorny
            case 1: center.x = r; center.y = r; break;                       // Lewy gorny
            case 2: center.x = r; center.y = size_.y - r; break;             // Lewy dolny
            case 3: center.x = size_.x - r; center.y = size_.y - r; break;   // Prawy dolny
            }

            float angle = deltaAngle * (index % cornerPointCount_) + (centerIndex * 90.0f);
            float rad = angle * 3.141592654f / 180.0f;

            return {
                center.x + r * std::cos(rad),
                center.y - r * std::sin(rad)
            };
        }

    private:
        sf::Vector2f size_;
        float radius_;
        std::size_t cornerPointCount_;
    };
}