#include "PauseState.h"
#include <iostream>

namespace game::states
{
    PauseState::PauseState(game::Game* game) : State(game)
    {
        sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();

        // 1. dark overlay
        darkOverlay.setSize(viewSize);
        darkOverlay.setFillColor(sf::Color(0, 0, 0, 180));

        try {
            // loading font
            uiFont.emplace("../../../assets/fonts/Minecraftia-Regular.ttf");

            
            confirmText.emplace(*uiFont);
            confirmText->setString("Are you sure you want to exit?");
            confirmText->setCharacterSize(36);
            confirmText->setFillColor(sf::Color::White);

            sf::FloatRect textBounds = confirmText->getLocalBounds();
            confirmText->setOrigin({ textBounds.size.x / 2.0f, textBounds.size.y / 2.0f });
            confirmText->setPosition({ viewSize.x / 2.0f, viewSize.y / 2.0f - 100.0f });
        }
        catch (const std::exception& e) {
            std::cerr << "[UI ERROR] Nie mozna zaladowac czcionki: " << e.what() << '\n';
        }

        float centerX = viewSize.x / 2.0f;
        float centerY = viewSize.y / 2.0f;

        setupButton("resume_btn", resumeTex, resumeBtn, { centerX, centerY - 80.0f }, { 200.0f, 60.0f });
        setupButton("settings_btn", settingsTex, settingsBtn, { centerX, centerY }, { 200.0f, 60.0f });
        setupButton("exit_btn", exitTex, exitBtn, { centerX, centerY + 80.0f }, { 200.0f, 60.0f });

        setupButton("yes_btn", yesTex, yesBtn, { centerX - 120.0f, centerY }, { 150.0f, 60.0f });
        setupButton("no_btn", noTex, noBtn, { centerX + 120.0f, centerY }, { 150.0f, 60.0f });
    }

    void PauseState::setupButton(const std::string& key, std::optional<sf::Texture>& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
    {
        if (game->menuUiBuffer.contains(key))
        {
            try
            {
                tex.emplace(game->menuUiBuffer.at(key));
                spr.emplace(*tex);

                sf::Vector2f originalSize(tex->getSize());
                float scaleX = targetSize.x / originalSize.x;
                float scaleY = targetSize.y / originalSize.y;
                spr->setScale({ scaleX, scaleY });
                spr->setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });
                spr->setPosition(pos);
            }
            catch (const std::exception& e)
            {
                std::cerr << "[UI ERROR] Creatig texture error '" << key << "': " << e.what() << '\n';
            }
        }
        else
        {
            std::cerr << "[UI ERROR] can not to find a button '" << key << "' w menuUiBuffer\n";
        }
    }

    StateType PauseState::getType() const
    {
        return StateType::Pause;
    }

    void PauseState::handleEvent(const sf::Event& event)
    {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                if (showExitConfirm) {
                    showExitConfirm = false; // reset question
                }
                else {
                    game->getStateMachine().popState(); // return game
                }
                return;
            }
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
                sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());

                if (showExitConfirm)
                {
                    if (yesBtn.has_value() && yesBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        game->getStateMachine().changeState(StateType::Menu);
                        return;
                    }
                    if (noBtn.has_value() && noBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        showExitConfirm = false; // return pause screen
                        return;
                    }
                }
                else
                {
                    if (resumeBtn.has_value() && resumeBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        game->getStateMachine().popState();
                        return;
                    }
                    if (settingsBtn.has_value() && settingsBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        game->getStateMachine().pushState(StateType::Settings);
                        return;
                    }
                    if (exitBtn.has_value() && exitBtn->getGlobalBounds().contains(worldPos)) {
                        game->playUIClick();
                        showExitConfirm = true;
                        return;
                    }
                }
            }
        }
    }

    void PauseState::update(float dt)
    {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
        sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());

        if (showExitConfirm) {
            updateHover(yesBtn, { 150.0f, 60.0f }, mousePos);
            updateHover(noBtn, { 150.0f, 60.0f }, mousePos);
        }
        else {
            updateHover(resumeBtn, { 200.0f, 60.0f }, mousePos);
            updateHover(settingsBtn, { 200.0f, 60.0f }, mousePos);
            updateHover(exitBtn, { 200.0f, 60.0f }, mousePos);
        }
    }

    void PauseState::updateHover(std::optional<sf::Sprite>& btn, sf::Vector2f targetSize, sf::Vector2f mousePos)
    {
        if (!btn) return;

        sf::Vector2f texSize(btn->getTexture().getSize());
        float baseScaleX = targetSize.x / texSize.x;
        float baseScaleY = targetSize.y / texSize.y;

        if (btn->getGlobalBounds().contains(mousePos)) {
            btn->setColor(sf::Color(255, 255, 255));
            btn->setScale({ baseScaleX * 1.1f, baseScaleY * 1.1f });
        }
        else {
            btn->setColor(sf::Color(210, 210, 210));
            btn->setScale({ baseScaleX, baseScaleY });
        }
    }

    void PauseState::render(sf::RenderWindow& window)
    {
        window.setView(window.getDefaultView());

        window.draw(darkOverlay);

        if (showExitConfirm) {
            if (confirmText.has_value()) window.draw(*confirmText);
            if (yesBtn.has_value()) window.draw(*yesBtn);
            if (noBtn.has_value()) window.draw(*noBtn);
        }
        else {
            if (resumeBtn.has_value()) window.draw(*resumeBtn);
            if (settingsBtn.has_value()) window.draw(*settingsBtn);
            if (exitBtn.has_value()) window.draw(*exitBtn);
        }
    }
}