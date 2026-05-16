#pragma once

#include "../core/State.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

namespace game::states
{
	// Pomocniczy enum do œledzenia, który klawisz w³aœnie przypisujemy
	enum class RebindTarget { None, Up, Left, Down, Right };

	class SettingsState : public State
	{
	private:
		// --- OVERLAY ---
		sf::RectangleShape darkOverlay;
		sf::Texture bgTexture;
		std::optional<sf::Sprite> bgSprite;

		// --- FONT ---
		sf::Font font;

		// --- AUDIO SLIDER ---
		std::optional<sf::Text> audioTitle;
		std::optional<sf::Text> volumeLabel;
		std::optional<sf::Text> volumeValueText;

		sf::RectangleShape sliderTrack;
		sf::RectangleShape sliderHandle;
		bool isDraggingSlider = false;
		float currentVolume = 100.0f;

		// --- (WSAD) ---
		std::optional<sf::Text> controlsTitle;

		std::optional<sf::Text> upLabel;      std::optional<sf::Text> upBtnText;
		std::optional<sf::Text> leftLabel;    std::optional<sf::Text> leftBtnText;
		std::optional<sf::Text> downLabel;    std::optional<sf::Text> downBtnText;
		std::optional<sf::Text> rightLabel;   std::optional<sf::Text> rightBtnText;

		RebindTarget currentRebind = RebindTarget::None;

		// --- NAVIGATE ---
		//std::optional<sf::Text> backBtnText;

		//backbutton
		sf::Texture backBtnTex;
		std::optional<sf::Sprite> backBtnSprite;

		// helpers
		std::string keyToString(sf::Keyboard::Key key);

	public:
		SettingsState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}