#pragma once

#include "../core/State.h"
#include <SFML/Graphics.hpp>
#include <deque> // <-- U¿ywamy bezpiecznej kolejki!
#include <vector>
#include <optional>
#include <string>
#include <cstdint>

namespace game::states
{
	class MapSelectState : public State
	{
	private:
		sf::Texture bgTex;
		std::optional<sf::Sprite> bgSprite;
		sf::RectangleShape darkOverlay;

		sf::Texture leftArrowTex, rightArrowTex;
		std::optional<sf::Sprite> leftArrowSprite, rightArrowSprite;

		sf::Texture selectBtnTex, backBtnTex;
		std::optional<sf::Sprite> selectBtnSprite, backBtnSprite;

		sf::Font font;
		std::optional<sf::Text> mapNameText;
		std::optional<sf::Text> mapDescText;
		std::optional<sf::Text> mapStatsText;

		sf::Clock animationClock;

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

		struct MapOption {
			std::string jsonKey;
			std::string name;
			std::string description;

			sf::Texture thumbnailTexture;
			std::optional<sf::Sprite> thumbnailSprite;

			int difficultyStars = 1;
			float damageMultiplier = 1.0f;
		};

		// --- BEZPIECZNA STRUKTURA DANYCH ---
		std::deque<MapOption> roster;

		int targetIndex;
		float currentScroll;

		void setupButton(const std::string& key, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize);
		void loadRoster();
		std::string getDifficultyStars(int stars);

	public:
		MapSelectState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}