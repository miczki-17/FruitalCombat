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

		// tmp ctr speed limit
		float speedUncapTimer = 0.0f;

		int hp = 100;
		int maxHp = 100;

		sf::Texture playerTexture;
		std::optional<sf::Sprite> playerSprite;
		sf::CircleShape shape;

		// --- NOWE: Wymiary pojedynczej klatki z arkusza (64x64) ---
		const sf::Vector2i frameSize = { 64, 64 };

		// --- Sloty umiej?tno?ci ---
		std::unique_ptr<game::components::Ability> primaryWeapon;
		std::unique_ptr<game::components::Ability> specialSkill;

		void handleMovement(float dt, game::Game* game, sf::Vector2f mapLimits, const sf::Image& collisionMask, float mapScale);

	public:
		Player();

		// Settery u?ywane przez Fabryk?
		void setStats(int newHp, float newMaxSpeed);
		void loadTexture(const std::string& filepath);
		void setWeapon(std::unique_ptr<game::components::Ability> weapon);
		void setSkill(std::unique_ptr<game::components::Ability> skill);

		// Dynamiczna modyfikacja fizyki (u?ywana m.in. przez Dash)
		void addVelocity(sf::Vector2f force, float uncapDuration = 0.0f);

		// Delegacja akcji
		void useWeapon(sf::Vector2f targetWorldPos);
		void useSkill(sf::Vector2f targetWorldPos);

		void update(float dt, game::Game* game, sf::Vector2f mapLimits, const sf::Image& collisionMask, float mapScale);
		void render(sf::RenderWindow& window);

		sf::Vector2f getPosition() const;
		sf::Vector2f getVelocity() const;
		void setPosition(sf::Vector2f pos);
	};
}