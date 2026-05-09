#include "StateMachine.h"

#include "../states/IntroState.h"
#include "../states/MenuState.h"
#include "../states/PlayingState.h"
#include "../states/SettingsState.h"


namespace game
{
	StateMachine::StateMachine(Game* game)
		: game(game)
	{
	}

	std::unique_ptr<states::State> StateMachine::createState(states::StateType type)
	{
		switch (type)
		{
		case states::StateType::Intro:
			return std::make_unique<states::IntroState>(game);

		case states::StateType::Menu:
			return std::make_unique<states::MenuState>(game);

		case states::StateType::Playing:
			return std::make_unique<states::PlayingState>(game);

		case states::StateType::Settings:
			return std::make_unique<states::SettingsState>(game);

		default:
			return nullptr;
		}
	}



	void StateMachine::changeState(states::StateType type)
	{
		stateStack.clear();
		stateStack.push_back(createState(type));
	}

	void StateMachine::handleEvent(const sf::Event& event)
	{
		if (!stateStack.empty())
		{
			stateStack.back()->handleEvent(event);
		}
	}

	void StateMachine::update(float dt)
	{
		if (!stateStack.empty())
		{
			stateStack.back()->update(dt);
		}
	}

	void StateMachine::render(sf::RenderWindow& window)
	{
		for (const auto& state : stateStack)
		{
			if (state)
			{
				state->render(window);
			}
		}
	}

	void StateMachine::pushState(states::StateType type)
	{
		stateStack.push_back(createState(type));
	}

	void StateMachine::popState()
	{
		if (!stateStack.empty())
		{
			stateStack.pop_back();
		}
	}

}