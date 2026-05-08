#pragma once

#include <memory>

#include "../core/State.h"


namespace game::states
{
	class MenuState : public State
	{
	public:
		MenuState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}