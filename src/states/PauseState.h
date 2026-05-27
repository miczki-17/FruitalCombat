// ==========================================
// PauseState.h
// ==========================================
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

        void setupButton(const std::string& key, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize);
        void updateHover(std::optional<sf::Sprite>& btn, sf::Vector2f targetSize, sf::Vector2f mousePos, std::optional<sf::Text>* linkedText = nullptr);
        void setupButtonText(std::optional<sf::Text>& textObj, const std::string& str, sf::Vector2f pos, int fontSize = 24);

    public:
        PauseState(game::Game* game);

        StateType getType() const override;
        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}