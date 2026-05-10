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

namespace game
{
	class Game
	{
	private:
		sf::RenderWindow window;
		StateMachine stateMachine;

	public:
		Game();

		void run();

		// UI
		void playUIClick();

		StateMachine& getStateMachine();
		sf::RenderWindow& getWindow();

		// pre-loading data buffers
		std::map<std::string, sf::Image> menuUiBuffer;
		std::vector<sf::Image> menuImageBuffer;

		// --- WSAD ---
		sf::Keyboard::Key keyUp = sf::Keyboard::Key::W;
		sf::Keyboard::Key keyLeft = sf::Keyboard::Key::A;
		sf::Keyboard::Key keyDown = sf::Keyboard::Key::S;
		sf::Keyboard::Key keyRight = sf::Keyboard::Key::D;

		// sounds
		sf::SoundBuffer uiClickBuffer;
		std::optional<sf::Sound> uiClickSound;

		// --- ZMIENNA KOMUNIKACYJNA: WYBÓR BOHATERA ---
		// Domyœlnie ustawiamy Jab³ko. Menu podmieni tê wartoœæ przed startem.
		game::entities::FruitType selectedFruitType = game::entities::FruitType::Apple;
	};
}