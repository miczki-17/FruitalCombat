// --- StateMachine.cpp ---


#include "StateMachine.h"
#include "../states/IntroState.h"
#include "../states/MenuState.h"
#include "../states/PlayingState.h"
#include "../states/SettingsState.h"
#include "../states/CharacterSelectState.h"
#include "../states/MapSelectState.h"
#include "../states/PauseState.h"

namespace game
{
	StateMachine::StateMachine(game::Game* game) : game(game) {}

	StateMachine::~StateMachine() = default;

	std::unique_ptr<states::State> StateMachine::createState(states::StateType type)
	{
		switch (type)
		{
		case states::StateType::Intro:				return std::make_unique<states::IntroState>(game);
		case states::StateType::Menu:				return std::make_unique<states::MenuState>(game);
		case states::StateType::CharacterSelect:	return std::make_unique<states::CharacterSelectState>(game);
		case states::StateType::Playing:			return std::make_unique<states::PlayingState>(game);
		case states::StateType::Settings:			return std::make_unique<states::SettingsState>(game);
		case states::StateType::MapSelect:			return std::make_unique<states::MapSelectState>(game);
		case states::StateType::Pause:				return std::make_unique<states::PauseState>(game);	
		default: return nullptr;
		}
	}

	// buff
	void StateMachine::changeState(states::StateType type)
	{
		pendingAction = Action::Change;
		pendingStateType = type;
	}

	void StateMachine::pushState(states::StateType type)
	{
		pendingAction = Action::Push;
		pendingStateType = type;
	}

	void StateMachine::popState()
	{
		pendingAction = Action::Pop;
	}

	// MEMORY CHANGE
	void StateMachine::processStateChanges()
	{
		if (pendingAction == Action::None) return;

		switch (pendingAction)
		{
		case Action::Change:
			stateStack.clear();
			stateStack.push_back(createState(pendingStateType));
			break;

		case Action::Push:
			stateStack.push_back(createState(pendingStateType));
			break;

		case Action::Pop:
			if (!stateStack.empty()) stateStack.pop_back();
			break;

		default: break;
		}

		pendingAction = Action::None;
	}

	// EVENTS
	void StateMachine::handleEvent(const sf::Event& event)
	{
		if (!stateStack.empty()) stateStack.back()->handleEvent(event);
	}

	// UPDATE
	void StateMachine::update(float dt)
	{
		if (!stateStack.empty()) stateStack.back()->update(dt);
	}

	// RENDER
	void StateMachine::render(sf::RenderWindow& window)
	{
		for (size_t i = 0; i < stateStack.size(); ++i)
		{
			if (stateStack[i]) stateStack[i]->render(window);
		}
	}


	// get last state
	states::StateType StateMachine::getCurrentStateType() const
	{
		if (stateStack.empty()) return states::StateType::Intro; // Default state
		return stateStack.back()->getType();
	}	
}