// --- MathUtils.h ---

#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace game::utils::math
{
    // return squared distance between two points
    template<typename T>
    inline float distanceSquared(const sf::Vector2<T>& a, const sf::Vector2<T>& b)
    {
        return (a - b).lengthSquared();
    }

	// return distance between two points
    template<typename T>
    inline float distance(const sf::Vector2<T>& a, const sf::Vector2<T>& b)
    {
        return (a - b).length();
    }


	// safeNormalize: returns a normalized vector, but if the input vector is too small (length <= epsilon), it returns a zero vector instead to avoid instability
    template<typename T>
    inline sf::Vector2<T> safeNormalize(const sf::Vector2<T>& vec, float epsilon = 0.001f)
    {
        if (vec.lengthSquared() <= epsilon * epsilon)
        {
			// if the vector is too small, return a zero vector to avoid instability
            return sf::Vector2<T>(0.f, 0.f);
        }

		// return the normalized vector if it's of sufficient length
        return vec.normalized();
    }
}