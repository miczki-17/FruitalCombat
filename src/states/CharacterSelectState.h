#pragma once

#include "../core/State.h"
#include "../entities/EntityTypes.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <string>

namespace game::states
{
	class CharacterSelectState : public State
	{
	private:
		// T³o i interfejs
		sf::Texture bgTex;
		std::optional<sf::Sprite> bgSprite;

		sf::Texture leftArrowTex, rightArrowTex;
		std::optional<sf::Sprite> leftArrowSprite, rightArrowSprite;

		sf::Texture selectBtnTex, backBtnTex;
		std::optional<sf::Sprite> selectBtnSprite, backBtnSprite;

		// Czcionka do nazw i statystyk
		sf::Font font;
		std::optional<sf::Text> characterNameText;

		// Struktura przechowuj¹ca dane i sprite'y dla ka¿dego owocu
		struct FruitOption {
			game::entities::FruitType type;
			std::string jsonKey;
			std::string displayName;
			sf::Texture texture;
			std::optional<sf::Sprite> sprite;
			int hp = 0;
			int damage = 0;
			int speed = 0;
		};

		std::vector<FruitOption> roster;

		// Logika animowanej karuzeli
		int targetIndex;         // Indeks, do którego zmierzamy
		float currentScroll;     // Aktualna p³ynna pozycja przewijania

		// Metody pomocnicze
		void setupButton(const std::string& path, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize);
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