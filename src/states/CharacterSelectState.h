#pragma once

#include "../core/State.h"
#include "../entities/EntityTypes.h"
#include <SFML/Graphics.hpp>
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

		// --- TYPOGRAPHY ---
		sf::Font font;
		std::optional<sf::Text> characterNameText;
		std::optional<sf::Text> characterTitleText;

		sf::Clock animationClock;

		// --- STRUCTURE FOR MAGICAL FIREFLIES ---
		struct Firefly {
			sf::Vector2f position;
			float speed = 0.0;
			float alpha = 0.0;
			float lifetime = 0.0;
			float maxLifetime = 0.0;
			float size = 0.0;
			float swayOffset = 0.0;
		};
		std::vector<Firefly> fireflies;
		void initFireflies();
		void updateFireflies(float dt);

		// --- CHARACTER/FRUIT DATA STRUCTURE ---
		struct FruitOption {
			// Assign a default enum value (resolves previous compilation warnings/errors)
			game::entities::FruitType type = game::entities::FruitType::Apple;

			std::string jsonKey;
			std::string displayName;
			std::string title;

			// Character texture and sprite
			sf::Texture texture;
			std::optional<sf::Sprite> sprite;

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

		// Carousel properties
		std::vector<FruitOption> roster;
		int targetIndex;
		float currentScroll;

		// --- HELPER METHODS ---
		void setupButton(const std::string& key, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize);
		void loadRoster();
		void drawStatBar(sf::RenderWindow& window, const std::string& label, int value, int maxValue, sf::Vector2f pos, sf::Color color);

	public:
		CharacterSelectState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}