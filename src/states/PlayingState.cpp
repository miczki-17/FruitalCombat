#include "PlayingState.h"

namespace game::states
{
	PlayingState::PlayingState(game::Game* game)
		: State(game)
	{
	}


	StateType PlayingState::getType() const
	{
		return StateType::Playing;
	}

	void PlayingState::handleEvent(const sf::Event& event)
	{

	}

	void  PlayingState::update(float dt)
	{

	}

	void PlayingState::render(sf::RenderWindow& window)
	{

	}
}
