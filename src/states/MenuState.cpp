#include "MenuState.h"

namespace game::states
{
	MenuState::MenuState(game::Game* game)
		: State(game)
	{
	}


	StateType MenuState::getType() const
	{
		return StateType::Menu;
	}

	void MenuState::handleEvent(const sf::Event& event)
	{

	}

	void  MenuState::update(float dt)
	{

	}

	void MenuState::render(sf::RenderWindow& window)
	{

	}
}
