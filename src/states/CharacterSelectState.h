// --- CharacterSelectState.h ---


#pragma once

#include "State.h"
#include "../entities/EntityTypes.h"
#include <SFML/Graphics.hpp>
#include <deque>
#include <vector>
#include <optional>
#include <string>
#include <cstdint>

namespace game::states
{
	class CharacterSelectState : public State
	{
	private:
		// --- BACKGROUND AND OVERLAY ---
		sf::Texture bgTex;
		std::optional<sf::Sprite> bgSprite;
		sf::RectangleShape darkOverlay;

		// --- UI BUTTONS ---
		sf::Texture leftArrowTex, rightArrowTex;
		std::optional<sf::Sprite> leftArrowSprite, rightArrowSprite;

		sf::Texture selectBtnTex, backBtnTex;
		std::optional<sf::Sprite> selectBtnSprite, backBtnSprite;

		// --- DYNAMIC TEXTS ---
		std::optional<sf::Text> selectBtnText;

		// --- UI ICONS ---
		sf::Texture hpIconTex, dmgIconTex, spdIconTex;
		std::optional<sf::Sprite> hpIconSprite, dmgIconSprite, spdIconSprite;

		// --- STAT BARS ---
		sf::Texture statBarFrameTex;
		std::optional<sf::Sprite> statBarFrameSprite;

		sf::Texture statBarFillTex;
		std::optional<sf::Sprite> statBarFillSprite;

		// --- TYPOGRAPHY ---
		std::optional<sf::Text> characterNameText;
		std::optional<sf::Text> characterTitleText;

		// OPTIMIZATION: std::optional is required in SFML 3 because sf::Text has no default constructor
		std::optional<sf::Text> abilitiesTextDisplay;

		sf::Clock animationClock;

		// --- STRUCTURE FOR MAGICAL FIREFLIES ---
		struct Firefly {
			sf::Vector2f position;
			float speed = 0.0f;
			float alpha = 0.0f;
			float lifetime = 0.0f;
			float maxLifetime = 0.0f;
			float size = 0.0f;
			float swayOffset = 0.0f;
		};
		std::vector<Firefly> fireflies;
		void initFireflies();
		void updateFireflies(float dt);

		// --- CHARACTER/FRUIT DATA STRUCTURE ---
		struct FruitOption {
			game::entities::FruitType type = game::entities::FruitType::Apple;

			std::string jsonKey;
			std::string displayName;
			std::string title;

			// Character texture and sprite
			sf::Texture texture;
			std::optional<sf::Sprite> sprite;

			// --- START ANIMATION (LOBBY START) ---
			sf::Texture startTexture;
			std::vector<sf::IntRect> startAnimationFrames;
			bool hasStartAnimation = false;
			bool isPlayingStartAnimation = false;
			bool hasPlayedStartAnimation = false;
			// ---------------------------------------

			// Platform texture and sprite
			sf::Texture platformTexture;
			std::optional<sf::Sprite> platformSprite;

			// Animation properties
			bool isAnimated = false;
			std::vector<sf::IntRect> animationFrames;
			int currentFrameIndex = 0;
			float animationTimer = 0.0f;

			// Character stats
			int hp = 0;
			int damage = 0;
			int speed = 0;
			std::string abilitiesText;
		};

		// --- ROSTER DATA ---
		std::deque<FruitOption> roster;

		int targetIndex;
		float currentScroll;

		// OPTIMIZATION: Pre-allocated vector for z-order sorting to avoid allocations in render loop
		std::vector<std::pair<float, int>> renderZOrder;

		// --- HELPER METHODS ---
		void setupButton(const std::string& key, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize);
		void loadRoster();
		void drawStatBar(sf::RenderWindow& window, std::optional<sf::Sprite>& icon, int value, float gameMaxValue, sf::Vector2f pos, sf::Color color, sf::Vector2f iconOffset, const std::string& labelText);

	public:
		CharacterSelectState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}