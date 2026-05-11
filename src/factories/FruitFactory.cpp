#include "FruitFactory.h"
#include "../components/ShootAbility.h"
#include "../components/DashAbility.h"

namespace game::factories
{
	FruitFactory::FruitFactory(game::ArenaContext arenaContext)
		: context(arenaContext)
	{
	}

	std::unique_ptr<game::entities::Player> FruitFactory::createFruit(game::entities::FruitType type)
	{
		auto player = std::make_unique<game::entities::Player>();

		switch (type)
		{
		case game::entities::FruitType::Apple:
			player->setStats(10, 400.0f);
			player->loadTexture("../../../assets/textures/entities/apple_player.png");
			player->setAbility(std::make_unique<game::components::ShootAbility>(context.bullets));
			break;

		case game::entities::FruitType::Banana:
			player->setStats(10, 550.0f);
			player->loadTexture("../../../assets/textures/entities/banana_player.png");
			player->setAbility(std::make_unique<game::components::DashAbility>(player.get()));
			break;

		case game::entities::FruitType::Orange:
			player->setStats(15, 450.0f);
			player->loadTexture("../../../assets/textures/entities/orange_player.png");
			// Domyœlnie wpinamy strzelanie, mo¿esz podmieniæ na inn¹ zdolnoœæ
			player->setAbility(std::make_unique<game::components::ShootAbility>(context.bullets));
			break;

		default:
			break;
		}

		return player;
	}
}