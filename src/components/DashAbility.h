#pragma once

// includes
#include "Ability.h"

namespace game::entities { class Player; }

namespace game::components
{
	// CLASS
	class DashAbility : public Ability
	{
	private:
		// player pointer
		game::entities::Player* player;

		// params
		float cooldown = 2.0f;
		float currentTimer = 0.0f;
		float dashForce = 1200.0f;

	public:
		// constructor
		DashAbility(game::entities::Player* targetPlayer);

		void update(float dt) override;
		void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
	};
}