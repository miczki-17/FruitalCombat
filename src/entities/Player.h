#pragma once

#include <SFML/Graphics.hpp>


namespace game::entities
{
	class Player
	{
	public:
		virtual ~Player() = default;

		virtual void handleEvent(const sf::Event& event) = 0;
		virtual void update(const float& dt) = 0;
		virtual void render(sf::RenderWindow& window) = 0;
	};
}