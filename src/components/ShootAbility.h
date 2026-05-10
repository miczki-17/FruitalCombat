#pragma once
#include "Ability.h"
#include "Bullet.h"
#include <vector>

namespace game::components
{
	class ShootAbility : public Ability
	{
	private:
		std::vector<game::entities::Bullet>& bullets;
		float cooldown = 0.2f;
		float currentTimer = 0.0f;

	public:
		ShootAbility(std::vector<game::entities::Bullet>& bulletsRef);

		void update(float dt) override;
		void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
	};
}