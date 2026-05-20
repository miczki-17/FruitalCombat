// --- ShootAbility.h --- 


#pragma once

// includes
#include "Ability.h"
#include "../projectiles/Bullet.h"
#include <vector>

namespace game::components
{
	// CLASS
	class ShootAbility : public Ability
	{
	private:
		// bullets vector
		std::vector<game::components::Bullet>& bullets;

		// bullet params
		float cooldown = 0.2f;
		float currentTimer = 0.0f;

	public:
		// constructor
		ShootAbility(std::vector<game::components::Bullet>& bulletsRef);

		void update(float dt) override;
		void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
	};
}