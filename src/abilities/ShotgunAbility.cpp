// --- ShotgunAbility.cpp ---


#include "ShotgunAbility.h"
#include <cmath>


// lack of constant
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace game::components
{
	// ability constructor
	ShotgunAbility::ShotgunAbility(std::vector<game::components::Bullet>& bulletsRef)
		: bullets(bulletsRef)
	{
	}

	// UPDATE
	void ShotgunAbility::update(float dt)
	{
		if (currentTimer > 0.0f)
		{
			currentTimer -= dt;
		}
	}

	// EXECUTE
	void ShotgunAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity)
	{
		if (currentTimer <= 0.0f)
		{
			sf::Vector2f aimDir = targetWorldPos - startPos;
			float length = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);

			if (length > 0.001f)
			{
				aimDir /= length;

				// 1. base shoot angle
				float baseAngle = std::atan2(aimDir.y, aimDir.x);

				// 2. conversion to radians
				float spreadRad = spreadAngle * (M_PI / 180.0f);
				float halfSpread = spreadRad / 2.0f;

				// 3. shoot bullets
				for (int i = 0; i < pelletCount; ++i)
				{
					// spread angles evenly
					float angleOffset = (pelletCount > 1)
						? (-halfSpread + (spreadRad * static_cast<float>(i) / (pelletCount - 1)))
						: 0.0f;

					float finalAngle = baseAngle + angleOffset;

					// new direction vector for bullet
					sf::Vector2f pelletDir(std::cos(finalAngle), std::sin(finalAngle));

					sf::Vector2f safeStartPos = startPos + (pelletDir * 30.0f);

					bullets.emplace_back(safeStartPos, pelletDir);
					bullets.back().addVelocity(shooterVelocity * 0.5f);

					bullets.back().setAppearance(6.2f, sf::Color(45, 25, 15));
				}

				// reset cooldown
				currentTimer = cooldown;
			}
		}
	}
}