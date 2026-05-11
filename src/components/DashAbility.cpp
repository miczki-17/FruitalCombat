#include "DashAbility.h"
#include "../entities/player/Player.h"
#include <cmath>

namespace game::components
{
	DashAbility::DashAbility(game::entities::Player* targetPlayer)
		: player(targetPlayer)
	{
	}

	void DashAbility::update(float dt)
	{
		if (currentTimer > 0.0f)
		{
			currentTimer -= dt;
		}
	}

	void DashAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f /*shooterVelocity*/)
	{
		if (currentTimer <= 0.0f && player != nullptr)
		{
			sf::Vector2f aimDir = targetWorldPos - startPos;
			float length = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);

			if (length > 0.001f)
			{
				aimDir /= length;

				// Gwa³townie dodajemy pêd do wektora prêdkoœci gracza
				player->addVelocity(aimDir * dashForce);

				currentTimer = cooldown;
			}
		}
	}
}