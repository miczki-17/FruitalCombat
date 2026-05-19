#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <map>
#include <string>
#include <vector>
#include <optional>

#include "Config.h"
#include "StateMachine.h"
#include "State.h"
#include "ArenaContext.h"

#include "../entities/EntityTypes.h"
#include "../vendor/nlohmann/json.hpp"

namespace game
{
	class Game
	{
	private:
		sf::RenderWindow window;
		StateMachine stateMachine;

		sf::Texture menuCursorTex;
		std::optional<sf::Sprite> menuCursorSprite;

		bool isCursorInitialized = false;

	public:
		Game();

		void run();

		// UI
		void playUIClick();

		// Cursor rendering
		void drawMenuCursor();

		StateMachine& getStateMachine();
		sf::RenderWindow& getWindow();

		// ---------- IMAGE BUFFERS ----------
		std::map<std::string, sf::Image> menuUiBuffer;
		std::vector<sf::Image> menuImageBuffer;
		std::map<std::string, sf::Image> characterImageBuffer;
		std::map<std::string, sf::Image> mapImageBuffer;

		// ---------- INPUT ----------
		sf::Keyboard::Key keyUp = sf::Keyboard::Key::W;
		sf::Keyboard::Key keyLeft = sf::Keyboard::Key::A;
		sf::Keyboard::Key keyDown = sf::Keyboard::Key::S;
		sf::Keyboard::Key keyRight = sf::Keyboard::Key::D;

		// ---------- AUDIO ----------
		sf::SoundBuffer uiClickBuffer;
		std::optional<sf::Sound> uiClickSound;

		sf::Music menuMusic;

		// ---------- PLAYER SELECTION ----------
		game::entities::FruitType selectedFruitType =
			game::entities::FruitType::Orange;

		std::string selectedMapKey = "WildOrchard";

		// ---------- CONFIGS ----------
		nlohmann::json fruitsConfig;
		nlohmann::json mapsConfig;

		// ---------- SHARED ARENA CONTEXT ----------
		ArenaContext arenaContext;
	};
}