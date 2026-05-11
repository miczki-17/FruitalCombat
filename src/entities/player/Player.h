#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include "../../components/Ability.h"
#include "../../core/Game.h"

namespace game::entities
{
	class Player
	{
	private:
		sf::Vector2f position;
		sf::Vector2f velocity;
		sf::Vector2f smoothedInput;

		float maxSpeed = 400.0f;
		float acceleration = 2500.0f;
		float activeDrag = 4.0f;
		float stopDrag = 8.0f;
		float turnSpeed = 15.0f;

		int hp = 100;
		int maxHp = 100;

		sf::Texture playerTexture;
		std::optional<sf::Sprite> playerSprite;
		sf::CircleShape shape;

		std::unique_ptr<game::components::Ability> primaryAbility;

		void handleMovement(float dt, game::Game* game, sf::Vector2f mapLimits, const sf::Image& collisionMask, float mapScale);

	public:
		Player();

		// Settery u¿ywane przez Fabrykê
		void setStats(int newHp, float newMaxSpeed);
		void loadTexture(const std::string& filepath);
		void setAbility(std::unique_ptr<game::components::Ability> newAbility);

		// Dynamiczna modyfikacja fizyki (u¿ywana m.in. przez Dash)
		void addVelocity(sf::Vector2f force);

		// Delegacja ataku
		void useAbility(sf::Vector2f targetWorldPos);

		void update(float dt, game::Game* game, sf::Vector2f mapLimits, const sf::Image& collisionMask, float mapScale);
		void render(sf::RenderWindow& window);

		sf::Vector2f getPosition() const;
		sf::Vector2f getVelocity() const;
		void setPosition(sf::Vector2f pos);
	};
}