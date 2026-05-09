#pragma once

#include <memory>
#include "State.h"

namespace game
{
    class Game; // forward

    class StateMachine
    {
    private:
        Game* game;

        std::vector<std::unique_ptr<states::State>> stateStack;
        std::unique_ptr<states::State> createState(states::StateType type);

    public:
        StateMachine(Game* game);

        // kill all states, set one
        void changeState(states::StateType type);

        // add new state
        void pushState(states::StateType type);

        //kill last state
        void popState();

        void handleEvent(const sf::Event& event);
        void update(float dt);
        void render(sf::RenderWindow& window);
    };
}