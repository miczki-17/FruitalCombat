#pragma once
#include "../core/State.h"
#include "../core/Game.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

namespace game::states
{
    class PauseState : public State
    {
    private:
        sf::RectangleShape darkOverlay;


        std::optional<sf::Texture> resumeTex;
        std::optional<sf::Texture> settingsTex;
        std::optional<sf::Texture> exitTex;

        std::optional<sf::Texture> yesTex;
        std::optional<sf::Texture> noTex;

        std::optional<sf::Sprite> resumeBtn, settingsBtn, exitBtn;
        std::optional<sf::Sprite> yesBtn, noBtn;

        std::optional<sf::Font> uiFont;
        std::optional<sf::Text> confirmText;

        bool showExitConfirm = false;

        void setupButton(const std::string& key, std::optional<sf::Texture>& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize);
        void updateHover(std::optional<sf::Sprite>& btn, sf::Vector2f targetSize, sf::Vector2f mousePos);

    public:
        PauseState(game::Game* game);

        StateType getType() const override;
        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}