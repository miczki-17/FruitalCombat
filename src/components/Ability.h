#pragma once
#include <SFML/Graphics.hpp>

namespace game::components
{
	class Ability
	{
	public:
		virtual ~Ability() = default;

		virtual void update(float dt) = 0;
		virtual void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) = 0;
	};
}