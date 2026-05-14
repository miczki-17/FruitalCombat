#include "DashAbility.h"
#include <cmath>
#include "../entities/player/Player.h"

namespace game::components
{
	// constructor
	DashAbility::DashAbility(game::entities::Player* playerRef)
		: player(playerRef)
	{
	}

	// UPDATE
	void DashAbility::update(float dt)
	{
		if (currentTimer > 0.0f)
		{
			currentTimer -= dt;
		}
	}

	// EXECUTE
	void DashAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity)
	{
		if (currentTimer <= 0.0f && player != nullptr)
		{
			//std::cout << "[DASH]!\n";

			sf::Vector2f dashDir = shooterVelocity;
			float currentSpeed = std::sqrt(dashDir.x * dashDir.x + dashDir.y * dashDir.y);

			if (currentSpeed > 10.0f)
			{
				dashDir /= currentSpeed;
			}
			else
			{
				dashDir = targetWorldPos - startPos;
				float aimLength = std::sqrt(dashDir.x * dashDir.x + dashDir.y * dashDir.y);

				if (aimLength > 0.001f) dashDir /= aimLength;
				else                    dashDir = { 1.0f, 0.0f };
			}

			player->addVelocity(dashDir * dashForce, 0.15f);

			// reset cooldown
			currentTimer = cooldown;
		}
	}
}