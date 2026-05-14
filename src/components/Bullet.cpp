#include "Bullet.h"

namespace game::components
{
	// constructor
	Bullet::Bullet(sf::Vector2f startPos, sf::Vector2f direction)
		: position(startPos)
	{
		// load default params
		shape.setRadius(bulletRadius);
		shape.setFillColor(bulletColor);
		shape.setOrigin({ 4.0f, 4.0f });
		shape.setPosition(position);

		velocity = direction * speed;
	}

	void Bullet::addVelocity(sf::Vector2f additionalVelocity)
	{
		velocity += additionalVelocity;
	}

	// UPDATE
	void Bullet::update(float dt, const sf::Image& collisionMask, float mapScale)
	{
		if (!isActive) return;

		position += velocity * dt;
		shape.setPosition(position);

		sf::Vector2u maskSize = collisionMask.getSize();
		int px = static_cast<int>(position.x / mapScale);
		int py = static_cast<int>(position.y / mapScale);

		if (px < 0 || px >= static_cast<int>(maskSize.x) || py < 0 || py >= static_cast<int>(maskSize.y) ||
			collisionMask.getPixel(sf::Vector2u(px, py)) == sf::Color::Black)
		{
			isActive = false;
		}
	}

	// RENDER
	void Bullet::render(sf::RenderWindow& window)
	{
		if (isActive)
		{
			window.draw(shape);
		}
	}

	void Bullet::destroy() { isActive = false; }
	bool Bullet::getIsActive() const { return isActive; }
	sf::Vector2f Bullet::getPosition() const { return position; }
	float Bullet::getRadius() const { return shape.getRadius(); }

	void Bullet::setAppearance(float radius, sf::Color color)
	{
		bulletRadius = radius;
		bulletColor = color;
		shape.setRadius(bulletRadius);
		shape.setFillColor(bulletColor);
	}
}