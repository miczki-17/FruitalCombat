#include "FruitFactory.h"
#include "../components/ShootAbility.h"
#include "../components/ShotgunAbility.h"
#include "../components/DashAbility.h"
#include <iostream>

namespace game::factories
{
	FruitFactory::FruitFactory(game::ArenaContext arenaContext, const nlohmann::json& jsonConfig)
		: context(arenaContext), config(jsonConfig)
	{
	}

	std::unique_ptr<game::entities::Player> FruitFactory::createFruit(game::entities::FruitType type)
	{
		auto player = std::make_unique<game::entities::Player>();

		std::string fruitKey;

		switch (type)
		{
		case game::entities::FruitType::Apple:
			fruitKey = "Apple";
			break;
		case game::entities::FruitType::Banana:
			fruitKey = "Banana";
			break;
		case game::entities::FruitType::Orange:
			fruitKey = "Orange";
			break;
		case game::entities::FruitType::Cherry:
			fruitKey = "Cherry";
		case game::entities::FruitType::Strawberry:
			fruitKey = "Strawberry";
		case game::entities::FruitType::Blackberry:
			fruitKey = "Blackberry";
		default:
			std::cerr << "can not find fruit type\n";
			return nullptr;
			break;
		}

		if (config.contains(fruitKey))
		{
			const auto& data = config[fruitKey];

			int hp = data["hp"].get<int>();
			float speed = data["maxSpeed"].get<float>();
			player->setStats(hp, speed);

			// load texture path
			std::string idleTexPath = data["texturePath"].get<std::string>();
			std::string walkTexPath = data["walkTexturePath"].get<std::string>();
			player->loadTextures(idleTexPath, walkTexPath);
			
			if (data.contains("abilities"))
			{
				for (const auto& abName : data["abilities"])
				{
					std::string type = abName.get<std::string>();
					if (type == "Shoot")        player->setWeapon(std::make_unique<game::components::ShootAbility>(context.bullets));
					else if (type == "Shotgun") player->setWeapon(std::make_unique<game::components::ShotgunAbility>(context.bullets));
					else if (type == "Dash")    player->setSkill(std::make_unique<game::components::DashAbility>(player.get())); // <--- POPRAWIONE NA setSkill
				}
			}
		}
		else
		{
			std::cerr << "[FABRYKA] Brak konfiguracji dla klucza: " << fruitKey << "\n";
		}

		return player;
	}
}
