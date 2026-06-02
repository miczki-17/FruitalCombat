// --- StateMacjine.h ---

#pragma once

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>

// forward declarations

namespace game
{
    class Game;
}

namespace game::states
{
    class State;
    enum class StateType;
}

namespace game
{
    class StateMachine final
    {
    public:
        explicit StateMachine(Game* game);
        ~StateMachine();

        void changeState(states::StateType type);
        void pushState(states::StateType type);
        void popState();

        void processStateChanges();

        void handleEvent(const sf::Event& event);
        void update(float dt);
        void render(sf::RenderWindow& window);

        states::StateType getCurrentStateType() const;

    private:
        enum class Action
        {
            None,
            Change,
            Push,
            Pop
        };

        Game* game_;

        std::vector<std::unique_ptr<states::State>> stack_;

        Action pendingAction_ = Action::None;
        states::StateType pendingStateType_{};

        std::unique_ptr<states::State> createState(
            states::StateType type);

        void applyChange();
    };
}