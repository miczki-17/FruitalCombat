#pragma once
#include <memory>
#include "../entities/player/Player.h"
#include "../entities/EntityTypes.h"
#include "../core/ArenaContext.h"

namespace game::factories
{
	class FruitFactory
	{
	private:
		game::ArenaContext context;
		const nlohmann::json& config;

	public:
		FruitFactory(game::ArenaContext arenaContext, const nlohmann::json& jsonConfig);

		std::unique_ptr<game::entities::Player> createFruit(game::entities::FruitType type);
	};
}