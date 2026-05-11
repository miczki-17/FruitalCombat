#pragma once
#include <SFML/Graphics.hpp>

namespace game::components
{
	class Bullet
	{
	private:
		sf::Vector2f position;
		sf::Vector2f velocity;
		float speed = 800.0f;
		bool isActive = true;

		sf::CircleShape shape;

	public:
		Bullet(sf::Vector2f startPos, sf::Vector2f direction);

		void update(float dt, const sf::Image& collisionMask, float mapScale);
		void render(sf::RenderWindow& window);

		void addVelocity(sf::Vector2f additionalVelocity);
		void destroy();

		bool getIsActive() const;
		sf::Vector2f getPosition() const;
		float getRadius() const;
	};
}