#include "StateMachine.h"

#include "../states/IntroState.h"
#include "../states/MenuState.h"
#include "../states/PlayingState.h"
#include "../states/SettingsState.h"
#include "../states/CharacterSelectState.h"
#include "../states/MapSelectState.h"
#include "../states/PauseState.h"
#include "../states/ShopState.h"

namespace game
{
    StateMachine::StateMachine(Game* game)
        : game_(game)
    {
    }

    StateMachine::~StateMachine() = default;

    void StateMachine::changeState(states::StateType type)
    {
        pendingAction_ = Action::Change;
        pendingStateType_ = type;
    }

    void StateMachine::pushState(states::StateType type)
    {
        pendingAction_ = Action::Push;
        pendingStateType_ = type;
    }

    void StateMachine::popState()
    {
        pendingAction_ = Action::Pop;
    }

    void StateMachine::processStateChanges()
    {
        if (pendingAction_ == Action::None)
        {
            return;
        }

        applyChange();
        pendingAction_ = Action::None;
    }

    void StateMachine::applyChange()
    {
        switch (pendingAction_)
        {
        case Action::Change:
            stack_.clear();
            stack_.push_back(createState(pendingStateType_));
            break;

        case Action::Push:
            stack_.push_back(createState(pendingStateType_));
            break;

        case Action::Pop:
            if (!stack_.empty())
            {
                stack_.pop_back();
            }
            break;

        default:
            break;
        }
    }

    void StateMachine::handleEvent(
        const sf::Event& event)
    {
        if (!stack_.empty() && stack_.back())
        {
            stack_.back()->handleEvent(event);
        }
    }

    void StateMachine::update(float dt)
    {
        if (!stack_.empty() && stack_.back())
        {
            stack_.back()->update(dt);
        }
    }

    void StateMachine::render(
        sf::RenderWindow& window)
    {
        for (auto& state : stack_)
        {
            if (state)
            {
                state->render(window);
            }
        }
    }

    states::StateType
        StateMachine::getCurrentStateType() const
    {
        if (stack_.empty())
        {
            return states::StateType::Intro;
        }

        return stack_.back()->getType();
    }

    std::unique_ptr<states::State>
        StateMachine::createState(
            states::StateType type)
    {
        switch (type)
        {
        case states::StateType::Intro:
            return std::make_unique<states::IntroState>(game_);

        case states::StateType::Menu:
            return std::make_unique<states::MenuState>(game_);

        case states::StateType::CharacterSelect:
            return std::make_unique<states::CharacterSelectState>(game_);

        case states::StateType::Playing:
            return std::make_unique<states::PlayingState>(game_);

        case states::StateType::Settings:
            return std::make_unique<states::SettingsState>(game_);

        case states::StateType::MapSelect:
            return std::make_unique<states::MapSelectState>(game_);

        case states::StateType::Pause:
            return std::make_unique<states::PauseState>(game_);

        case states::StateType::Shop:
            return std::make_unique<states::ShopState>(game_);
        }

        return nullptr;
    }
}