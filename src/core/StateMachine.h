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

        std::unique_ptr<states::State> currentState;
        std::unique_ptr<states::State> createState(states::StateType type);

    public:
        StateMachine(Game* game);

        void changeState(states::StateType type);

        void handleEvent(const sf::Event& event);
        void update(float dt);
        void render(sf::RenderWindow& window);
    };
}