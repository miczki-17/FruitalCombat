#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>

namespace game::states { class State; enum class StateType; }

namespace game
{
	class Game;

	class StateMachine
	{
	private:
		Game* game;
		std::vector<std::unique_ptr<states::State>> stateStack;

		// safty fsm change
		enum class Action { None, Push, Pop, Change };
		Action pendingAction = Action::None;
		states::StateType pendingStateType;

		std::unique_ptr<states::State> createState(states::StateType type);

	public:
		// constructor
		StateMachine(Game* game);

		// destructor
		~StateMachine();

		void changeState(states::StateType type);
		void pushState(states::StateType type);
		void popState();

		void handleEvent(const sf::Event& event);
		void update(float dt);
		void render(sf::RenderWindow& window);

		
		void processStateChanges();
	};
}