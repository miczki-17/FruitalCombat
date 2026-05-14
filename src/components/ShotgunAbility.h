#pragma once

// includes
#include "Ability.h"
#include "Bullet.h"
#include <vector>

namespace game::components
{
	// CLASS
	class ShotgunAbility : public Ability
	{
	private:
		// bullets vector
		std::vector<game::components::Bullet>& bullets;

		// shotgun params
		float cooldown = 0.75f;
		float currentTimer = 0.0f;

		int pelletCount = 2;
		float spreadAngle = 7.5f;

	public:
		// abilty constructor
		ShotgunAbility(std::vector<game::components::Bullet>& bulletsRef);

		void update(float dt) override;
		void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
	};
}