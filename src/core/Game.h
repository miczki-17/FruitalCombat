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

		//cuursor
		void drawMenuCursor();

		StateMachine& getStateMachine();
		sf::RenderWindow& getWindow();

		// ----- BUFORY DANYCH (RAM) -----
		std::map<std::string, sf::Image> menuUiBuffer;
		std::vector<sf::Image> menuImageBuffer;
		std::map<std::string, sf::Image> characterImageBuffer; // BUFOR NA POSTACIE
		std::map<std::string, sf::Image> mapImageBuffer;


		// --- WSAD ---
		sf::Keyboard::Key keyUp = sf::Keyboard::Key::W;
		sf::Keyboard::Key keyLeft = sf::Keyboard::Key::A;
		sf::Keyboard::Key keyDown = sf::Keyboard::Key::S;
		sf::Keyboard::Key keyRight = sf::Keyboard::Key::D;

		// sounds
		sf::SoundBuffer uiClickBuffer;
		std::optional<sf::Sound> uiClickSound;

		// music
		sf::Music menuMusic;

		// default character
		game::entities::FruitType selectedFruitType = game::entities::FruitType::Apple;
		//default map & map key <-----V
		std::string selectedMapKey = "WildOrchard";

		// characters configs
		nlohmann::json fruitsConfig;
		// maps config
		nlohmann::json mapsConfig;
	};
}