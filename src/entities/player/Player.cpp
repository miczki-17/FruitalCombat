#include "Player.h"
#include <iostream>
#include <cmath>
#include <algorithm> // Wymagane dla std::clamp

namespace game::entities
{
	Player::Player()
	{
		position = { 400.0f, 300.0f };
	}

	void Player::initFruit(FruitType type)
	{
		switch (type)
		{
		case FruitType::Apple:
			std::cout << "[PLAYER] Zespawnowano jako: JAB£KO (P³ynny ruch + Granice mapy)\n";

			shape.setRadius(20.0f);
			shape.setFillColor(sf::Color::Red);

			// Obramowanie pozwala precyzyjnie widzieæ rotacjê kulki podczas ruchu
			shape.setOutlineThickness(5.0f);
			shape.setOutlineColor(sf::Color::Yellow);

			// Ustawienie punktu centralnego idealnie w œrodku
			shape.setOrigin({ 20.0f, 20.0f });
			break;

		default:
			break;
		}

		currentHealth = maxHealth;
		shape.setPosition(position);
	}

	void Player::handleMovement(float dt, game::Game* game, sf::Vector2f mapLimits)
	{
		// 1. Pobranie SUROWEJ intencji ruchu
		sf::Vector2f targetDir(0.f, 0.f);

		if (sf::Keyboard::isKeyPressed(game->keyUp))    targetDir.y -= 1.f;
		if (sf::Keyboard::isKeyPressed(game->keyDown))  targetDir.y += 1.f;
		if (sf::Keyboard::isKeyPressed(game->keyLeft))  targetDir.x -= 1.f;
		if (sf::Keyboard::isKeyPressed(game->keyRight)) targetDir.x += 1.f;

		if (targetDir.x != 0.f || targetDir.y != 0.f)
		{
			float length = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
			targetDir.x /= length;
			targetDir.y /= length;
		}

		// 2. INTERPOLACJA WEKTORA (LERP) - P³ynne wchodzenie w zakrêty
		smoothedInput += (targetDir - smoothedInput) * turnSpeed * dt;

		// 3. APLIKACJA SI£
		float smoothedLength = std::sqrt(smoothedInput.x * smoothedInput.x + smoothedInput.y * smoothedInput.y);

		if (smoothedLength > 0.05f)
		{
			velocity += smoothedInput * acceleration * dt;
			velocity -= velocity * activeDrag * dt;
		}
		else
		{
			velocity -= velocity * stopDrag * dt;

			float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
			if (currentSpeed < 10.0f)
			{
				velocity = { 0.f, 0.f };
			}
		}

		// 4. CLAMP PRÊDKOŒCI
		float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
		if (currentSpeed > maxSpeed)
		{
			velocity.x = (velocity.x / currentSpeed) * maxSpeed;
			velocity.y = (velocity.y / currentSpeed) * maxSpeed;
		}

		// 5. OBLICZENIE POTENCJALNEJ POZYCJI
		sf::Vector2f nextPos = position + velocity * dt;

		// 6. FIZYKA GRANIC (Clamping do krawêdzi PNG)
		// Uwzglêdniamy promieñ oraz gruboœæ obramowania, by kulka nie wchodzi³a w œciany
		float fullRadius = shape.getRadius() + shape.getOutlineThickness();

		nextPos.x = std::clamp(nextPos.x, fullRadius, mapLimits.x - fullRadius);
		nextPos.y = std::clamp(nextPos.y, fullRadius, mapLimits.y - fullRadius);

		// Zatrzymanie pêdu przy zderzeniu ze œcian¹
		if (nextPos.x == fullRadius || nextPos.x == mapLimits.x - fullRadius) velocity.x = 0.f;
		if (nextPos.y == fullRadius || nextPos.y == mapLimits.y - fullRadius) velocity.y = 0.f;

		position = nextPos;
	}

	void Player::update(float dt, game::Game* game, sf::Vector2f mapLimits)
	{
		handleMovement(dt, game, mapLimits);
		shape.setPosition(position);

		// --- OBRÓT W KIERUNKU RUCHU (SFML 3) ---
		float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
		if (currentSpeed > 5.0f)
		{
			shape.setRotation(sf::radians(std::atan2(velocity.y, velocity.x)));
		}
	}

	void Player::render(sf::RenderWindow& window)
	{
		window.draw(shape);
	}

	void Player::setPosition(sf::Vector2f pos)
	{
		position = pos;
		shape.setPosition(position);
	}

	sf::Vector2f Player::getPosition() const
	{
		return position;
	}
}