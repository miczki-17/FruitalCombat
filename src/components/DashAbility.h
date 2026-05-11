#pragma once
#include "Ability.h"

// Deklaracja zapowiadaj¹ca, by unikn¹æ pêtli nag³ówków
namespace game::entities { class Player; }

namespace game::components
{
	class DashAbility : public Ability
	{
	private:
		game::entities::Player* player; // WskaŸnik na cia³o, które pchniemy
		float cooldown = 1.0f;
		float currentTimer = 0.0f;
		float dashForce = 1200.0f;

	public:
		DashAbility(game::entities::Player* targetPlayer);

		void update(float dt) override;
		void execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity) override;
	};
}