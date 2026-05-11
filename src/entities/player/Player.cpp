#include "Player.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace game::entities
{
	Player::Player()
	{
		shape.setRadius(25.0f);
		shape.setOutlineThickness(0.0f);
	}

	void Player::setStats(int newHp, float newMaxSpeed)
	{
		hp = newHp;
		maxHp = newHp;
		maxSpeed = newMaxSpeed;
	}

	void Player::loadTexture(const std::string& filepath)
	{
		if (playerTexture.loadFromFile(filepath))
		{
			playerSprite.emplace(playerTexture);
			sf::Vector2u texSize = playerTexture.getSize();
			playerSprite->setOrigin({ texSize.x / 2.0f, texSize.y / 2.0f });
		}
		else
		{
			std::cerr << "[OSTRZE¯ENIE] Brak tekstury: " << filepath << "\n";
		}
	}

	void Player::setAbility(std::unique_ptr<game::components::Ability> newAbility)
	{
		primaryAbility = std::move(newAbility);
	}

	void Player::addVelocity(sf::Vector2f force)
	{
		velocity += force;
	}

	void Player::useAbility(sf::Vector2f targetWorldPos)
	{
		if (primaryAbility != nullptr)
		{
			// Przekazujemy nasz¹ pozycjê, cel oraz aktualn¹ prêdkoœæ (Doppler)
			primaryAbility->execute(position, targetWorldPos, velocity);
		}
	}

	void Player::handleMovement(float dt, game::Game* game, sf::Vector2f mapLimits, const sf::Image& collisionMask, float mapScale)
	{
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

		smoothedInput += (targetDir - smoothedInput) * turnSpeed * dt;

		float smoothedLength = std::sqrt(smoothedInput.x * smoothedInput.x + smoothedInput.y * smoothedInput.y);
		if (smoothedLength > 0.05f)
		{
			velocity += smoothedInput * acceleration * dt;
			velocity -= velocity * activeDrag * dt;
		}
		else
		{
			velocity -= velocity * stopDrag * dt;
			if (std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y) < 10.0f) velocity = { 0.f, 0.f };
		}

		float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
		if (currentSpeed > maxSpeed)
		{
			velocity.x = (velocity.x / currentSpeed) * maxSpeed;
			velocity.y = (velocity.y / currentSpeed) * maxSpeed;
		}

		// --- MAŒLANY POŒLIZG (SONAR WEKTOROWY) ---
		sf::Vector2f nextPos = position + velocity * dt;
		float fullRadius = shape.getRadius() + shape.getOutlineThickness();
		sf::Vector2u maskSize = collisionMask.getSize();

		sf::Vector2f collisionNormal(0.f, 0.f);
		sf::Vector2f pushVector(0.f, 0.f);
		int hitCount = 0;
		const int numProbes = 24;

		for (int i = 0; i < numProbes; ++i)
		{
			float angle = (i * 2.0f * 3.14159265f) / numProbes;
			sf::Vector2f offset(std::cos(angle) * fullRadius, std::sin(angle) * fullRadius);
			sf::Vector2f probePoint = nextPos + offset;

			int px = static_cast<int>(probePoint.x / mapScale);
			int py = static_cast<int>(probePoint.y / mapScale);

			bool isBlackPixel = false;
			if (px < 0 || px >= static_cast<int>(maskSize.x) || py < 0 || py >= static_cast<int>(maskSize.y))
			{
				isBlackPixel = true;
			}
			else
			{
				isBlackPixel = (collisionMask.getPixel(sf::Vector2u(px, py)) == sf::Color::Black);
			}

			if (isBlackPixel)
			{
				hitCount++;
				pushVector -= offset;
			}
		}

		if (hitCount > 0)
		{
			float pushLength = std::sqrt(pushVector.x * pushVector.x + pushVector.y * pushVector.y);
			if (pushLength > 0.001f)
			{
				collisionNormal = pushVector / pushLength;
				float dotProduct = velocity.x * collisionNormal.x + velocity.y * collisionNormal.y;

				if (dotProduct < 0.f)
				{
					velocity -= collisionNormal * dotProduct;
					velocity -= velocity * 0.03f;
				}
				nextPos = position + velocity * dt;
			}
		}

		for (int step = 0; step < 3; ++step)
		{
			sf::Vector2f currentPush(0.f, 0.f);
			int currentHits = 0;

			for (int i = 0; i < numProbes; ++i)
			{
				float angle = (i * 2.0f * 3.14159265f) / numProbes;
				sf::Vector2f offset(std::cos(angle) * fullRadius, std::sin(angle) * fullRadius);
				sf::Vector2f probePoint = nextPos + offset;

				int px = static_cast<int>(probePoint.x / mapScale);
				int py = static_cast<int>(probePoint.y / mapScale);

				if (px < 0 || px >= static_cast<int>(maskSize.x) || py < 0 || py >= static_cast<int>(maskSize.y) ||
					collisionMask.getPixel(sf::Vector2u(px, py)) == sf::Color::Black)
				{
					currentPush -= offset;
					currentHits++;
				}
			}

			if (currentHits == 0) break;

			float length = std::sqrt(currentPush.x * currentPush.x + currentPush.y * currentPush.y);
			if (length > 0.001f)
			{
				sf::Vector2f normal = currentPush / length;
				nextPos += normal * 0.6f;
			}
			else
			{
				nextPos.y += 0.5f;
			}
		}

		position = nextPos;
	}

	void Player::update(float dt, game::Game* game, sf::Vector2f mapLimits, const sf::Image& collisionMask, float mapScale)
	{
		handleMovement(dt, game, mapLimits, collisionMask, mapScale);

		if (playerSprite.has_value())
		{
			playerSprite->setPosition(position);

			sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
			sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getView());
			sf::Vector2f aimDir = mouseWorldPos - position;

			float angleDegrees = std::atan2(aimDir.y, aimDir.x) * 180.0f / 3.14159265f;

			// --- ZGODNOŒÆ Z SFML 3 (sf::degrees) ---
			playerSprite->setRotation(sf::degrees(angleDegrees + 90.0f));
		}
		else
		{
			shape.setPosition(position);
		}

		if (primaryAbility != nullptr)
		{
			primaryAbility->update(dt);
		}
	}

	void Player::render(sf::RenderWindow& window)
	{
		if (playerSprite.has_value())
		{
			window.draw(*playerSprite);
		}
		else
		{
			window.draw(shape);
		}
	}

	sf::Vector2f Player::getPosition() const { return position; }
	sf::Vector2f Player::getVelocity() const { return velocity; }
	void Player::setPosition(sf::Vector2f pos) { position = pos; }
}