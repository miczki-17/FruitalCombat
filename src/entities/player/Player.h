#pragma once

#include <SFML/Graphics.hpp>
#include "../EntityTypes.h"
#include "../../core/Game.h"

namespace game::entities
{
	class Player
	{
	private:
		sf::CircleShape shape;
		sf::Vector2f position;

		// --- FIZYKA P£YNNEGO RUCHU (SMOOTH GLIDE / DRIFT) ---
		sf::Vector2f velocity{ 0.f, 0.f };

		// Pamiêæ wirtualnego joysticka
		sf::Vector2f smoothedInput{ 0.f, 0.f };
		float turnSpeed = 50.0f;       // Jak p³ynnie wektor skrêca (mniejsza = szersze ósemki, wiêksza = ostrzejsze)

		float maxSpeed = 500.0f;
		float acceleration = 3500.0f; // Nieco ³agodniejszy zryw, ¿eby daæ przestrzeñ na pêd

		float activeDrag = 7.5f;     // Ma³e tarcie w ruchu = swobodne p³yniêcie w zakrêtach
		float stopDrag = 20.0f;       // £agodne, estetyczne wyhamowanie (skating)

		float maxHealth = 100.0f;
		float currentHealth = 100.0f;

	public:
		Player();

		void initFruit(FruitType type);
		void handleMovement(float dt, game::Game* game, sf::Vector2f mapLimits);
		void update(float dt, game::Game* game, sf::Vector2f mapLimits);
		void render(sf::RenderWindow& window);

		void setPosition(sf::Vector2f pos);
		sf::Vector2f getPosition() const;
	};
}