#include "StateMachine.h"

#include "../states/IntroState.h"
#include "../states/MenuState.h"
#include "../states/PlayingState.h"


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

		default:
			return nullptr;
		}
	}



	void StateMachine::changeState(states::StateType type)
	{
		currentState = createState(type);
	}

	void StateMachine::handleEvent(const sf::Event& event)
	{
		if (currentState)
			currentState->handleEvent(event);
	}

	void StateMachine::update(float dt)
	{
		if (currentState)
			currentState->update(dt);
	}

	void StateMachine::render(sf::RenderWindow& window)
	{
		if (currentState)
			currentState->render(window);
	}

}