#pragma once
#include <vector>
#include "../components/Bullet.h"

namespace game
{
	// Struktura przekazuj¹ca referencje do globalnych zasobów areny
	struct ArenaContext
	{
		std::vector<game::components::Bullet>& bullets;
		// W przysz³oœci dodasz tu np.: std::vector<Enemy>& enemies;
	};
}