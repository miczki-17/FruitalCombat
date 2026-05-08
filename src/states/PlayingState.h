#pragma once

#include <memory>

#include "../entities/Player.h"
#include "../core/State.h"


namespace game::states
{
	class PlayingState : public State
	{
	public:
		PlayingState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}