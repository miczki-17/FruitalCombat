#pragma once

#include "State.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <optional>

namespace game::states
{
    enum class RebindTarget { None, Up, Left, Down, Right, Fertilizer };

    class SettingsState : public State
    {
    private:
        sf::RectangleShape darkOverlay;
        sf::Texture bgTexture;
        std::optional<sf::Sprite> bgSprite;

        std::optional<sf::Text> settingsText;

        std::optional<sf::Sprite> langLeftArrow;
        std::optional<sf::Sprite> langRightArrow;

        // Rozmiary strza³ek do hovera
        sf::Vector2f arrowSize{ 40.0f, 40.0f };

        // --- LEFT SECTION: AUDIO & LANG ---
        std::optional<sf::Text> audioTitle;
        sf::RectangleShape sliderTrack;
        sf::RectangleShape sliderHandle;
        std::optional<sf::Text> volumeValueText;
        bool isDraggingSlider = false;
        float currentVolume = 100.0f;

        std::optional<sf::Text> langLabel;
        std::optional<sf::Text> langBtnText; // Nasz prze³¹cznik jêzyka

        // --- RIGHT SECTION: SCROLLABLE BINDS ---
        sf::RectangleShape bindsBackground; // Ciemne t³o dla listy
        sf::View scrollView; // Okienko kamery do przewijania
        float scrollOffset = 0.0f;
        float maxScroll = 0.0f;

        // --- SCROLLBAR ---
        sf::RectangleShape scrollbarTrack;
        sf::RectangleShape scrollbarThumb;
        bool isDraggingScrollbar = false;

        std::optional<sf::Text> controlsTitle;

        std::optional<sf::Text> upLabel;      std::optional<sf::Text> upBtnText;
        std::optional<sf::Text> leftLabel;    std::optional<sf::Text> leftBtnText;
        std::optional<sf::Text> downLabel;    std::optional<sf::Text> downBtnText;
        std::optional<sf::Text> rightLabel;   std::optional<sf::Text> rightBtnText;
        std::optional<sf::Text> fertilizerLabel; std::optional<sf::Text> fertilizerBtnText;

        RebindTarget currentRebind = RebindTarget::None;

        // --- BACK BUTTON ---
        std::optional<sf::Sprite> backBtnSprite;
        float baseBackScale = 1.0f;

        // --- HELPERS ---
        std::string keyToString(sf::Keyboard::Key key);
        void refreshTexts(); // odswierza napisy po zmianie jezyka
        void setupBindRow(std::optional<sf::Text>& label, std::optional<sf::Text>& btn, float yPos);
        
        void setupButton(const std::string& key, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize);
        void updateHover(std::optional<sf::Sprite>& btn, sf::Vector2f targetSize, sf::Vector2f mousePos, std::optional<sf::Text>* linkedText = nullptr);
        void alignTextRight(sf::Text& text);

    public:
        SettingsState(game::Game* game);
        StateType getType() const override { return StateType::Settings; }

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void render(sf::RenderWindow& window) override;
    };
}