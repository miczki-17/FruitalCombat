#include "ShootAbility.h"
#include <cmath>

namespace game::components
{
	ShootAbility::ShootAbility(std::vector<game::components::Bullet>& bulletsRef)
		: bullets(bulletsRef)
	{
	}

	void ShootAbility::update(float dt)
	{
		if (currentTimer > 0.0f)
		{
			currentTimer -= dt;
		}
	}

	void ShootAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity)
	{
		if (currentTimer <= 0.0f)
		{
			sf::Vector2f aimDir = targetWorldPos - startPos;
			float length = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);

			if (length > 0.001f)
			{
				aimDir /= length;

				bullets.emplace_back(startPos, aimDir);

				bullets.back().addVelocity(shooterVelocity);

				currentTimer = cooldown;
			}
		}
	}
}