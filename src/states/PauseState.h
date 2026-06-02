// --- PauseState.h ---

#pragma once
#include "State.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

namespace game::states
{
    class PauseState : public State
    {
    private:
        sf::RectangleShape darkOverlay;

        std::optional<sf::Sprite> resumeBtn, settingsBtn, exitBtn;
        std::optional<sf::Sprite> yesBtn, noBtn;

        std::optional<sf::Text> resumeText, settingsText, exitText, yesText, noText;
        std::optional<sf::Text> confirmText;

        bool showExitConfirm = false;

        void initUI();

        std::string lastLangCode;
        void refreshTexts();

    public:
        PauseState(game::Game* game);
        ~PauseState() override = default;

        StateType getType() const override { return StateType::Pause; }
        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}