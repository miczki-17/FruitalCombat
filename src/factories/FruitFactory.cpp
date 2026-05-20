#include "FruitFactory.h"

#include "../components/ShootAbility.h"
#include "../components/ShotgunAbility.h"
#include "../components/DashAbility.h"
#include "../components/AcidSquirtAbility.h"
#include "../components/RindRollAbility.h"

#include <iostream>

namespace game::factories
{
	FruitFactory::FruitFactory(
		game::ArenaContext& arenaContext,
		const nlohmann::json& jsonConfig)
		: context(arenaContext),
		config(jsonConfig)
	{
	}

	std::unique_ptr<game::entities::Player>
		FruitFactory::createFruit(game::entities::FruitType type)
	{
		auto player =
			std::make_unique<game::entities::Player>();

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
			break;

		case game::entities::FruitType::Strawberry:
			fruitKey = "Strawberry";
			break;

		case game::entities::FruitType::Blackberry:
			fruitKey = "Blackberry";
			break;

		default:
			std::cerr
				<< "[FACTORY ERROR] Unknown fruit type!\n";

			return nullptr;
		}

		if (config.contains(fruitKey))
		{
			const auto& data = config[fruitKey];

			int hp = data["hp"].get<int>();

			float speed =
				data["maxSpeed"].get<float>();

			float attackSpeed =
				data.value("attackSpeed", 1.0f);

			int idleFrames =
				data["idleFrames"].get<int>();
			
			int walkFrames =
				data["walkFrames"].get<int>();

			player->setStats(
				hp,
				speed,
				attackSpeed);

			// Load textures
			player->loadTextures(
				data["idleTexturePath"].get<std::string>(),
				idleFrames,
				data["walkTexturePath"].get<std::string>(),
				walkFrames);

			// Ability loading
			if (data.contains("abilities"))
			{
				for (const auto& abName : data["abilities"])
				{
					std::string abilityName =
						abName.get<std::string>();

					// Basic shoot
					if (abilityName == "Shoot")
					{
						player->setWeapon(
							std::make_unique<
							game::components::ShootAbility>(
								*context.bullets));
					}

					// Shotgun
					else if (abilityName == "Shotgun")
					{
						player->setWeapon(
							std::make_unique<
							game::components::ShotgunAbility>(
								*context.bullets));
					}

					// Dash
					else if (abilityName == "Dash")
					{
						player->setSkill(
							std::make_unique<
							game::components::DashAbility>(
								player.get()));
					}

					// Acid mortar projectile
					else if (abilityName == "AcidSquirt")
					{
						std::string texPath =
							data.value(
								"projectileTexture",
								"assets/textures/default_bullet.png");

						player->setWeapon(
							std::make_unique<
							game::components::AcidSquirtAbility>(
								*context.bullets,
								player.get(),
								texPath));
					}

					// Rolling skill
					else if (abilityName == "RindRoll")
					{
						float customRollDuration =
							data.value("rollDuration", 1.5f);

						float customRollSpeed =
							data.value("rollSpeed", 80.0f);

						player->setSkill(
							std::make_unique<
							game::components::RindRollAbility>(
								player.get(),
								customRollDuration,
								customRollSpeed));
					}
					else
					{
						std::cerr
							<< "[FACTORY WARNING] Unknown ability: "
							<< abilityName
							<< "\n";
					}
				}
			}
		}

		return player;
	}
}