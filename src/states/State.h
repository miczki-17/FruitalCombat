// -- State.h ---


// --- State.h ---

#pragma once

#include <iostream>
#include <optional>
#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace game
{
	class Game;
}


namespace game::states
{
	enum class StateType
	{
		Intro,
		Menu,
		Lobby,
		Playing,
		Settings,
		CharacterSelect,
		MapSelect,
		Pause,
		Shop,
		ShopInGame,
		Death
	};



	// CLASS
	class State
	{
	protected:
		game::Game* game;

		void setupButtonText(std::optional<sf::Text>& textObj, const sf::String& str, sf::Vector2f pos, int fontSize = 24);
		void setupButton(const std::string& key, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize);
		void updateHover(std::optional<sf::Sprite>& btn, sf::Vector2f targetSize, sf::Vector2f mousePos, std::optional<sf::Text>* linkedText = nullptr);

		const float GLOBAL_FONT_SCALE = 1.0f;

	public:
		// constructor
		State(game::Game* game)
			: game(game) {}


		// abstract destructor
		virtual ~State() = default;

		virtual void handleEvent(const sf::Event& event) = 0;
		virtual void update(float dt) = 0;
		virtual void render(sf::RenderWindow& window) = 0;

		virtual StateType getType() const = 0;
	};
}