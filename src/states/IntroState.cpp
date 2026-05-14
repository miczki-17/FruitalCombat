#include "IntroState.h"
#include "../core/Game.h"
#include <format>
#include "../vendor/nlohmann/json.hpp"
#include <fstream>
#include <iostream>

namespace game::states
{
	IntroState::IntroState(game::Game* game)
		: State(game), elapsedTime(0.f)
	{
		// Czas trwania jednej klatki dla wideo 30 FPS
		frameDuration = 1.0f / 30.0f;

		std::cout << "[INTRO] Initializing video intro...\n";

		// Wczytanie wideo (upewnij si?, ?e plik intro.mp4 znajduje si? w tym folderze)
		std::string videoPath = "assets/textures/intro/intro.mp4";

		if (!videoPlayer.load(videoPath))
		{
			std::cerr << "[INTRO ERROR] Failed to load intro video!\n";
		}

		// Dopasowanie rozmiaru wideo do okna
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		videoPlayer.fitToView(viewSize);

		// Uruchomienie ?adowania menu w osobnym w?tku
		workerThread = std::make_unique<std::thread>(&IntroState::loadAssetsInBg, this);

		// Odtwarzanie muzyki
		if (introMusic.openFromFile("assets/audio/intro/intro.mp3"))
		{
			introMusic.play();
		}
		else
		{
			std::cerr << "[INTRO ERROR] Cannot load intro music.\n";
		}
	}

	IntroState::~IntroState()
	{
		if (workerThread && workerThread->joinable())
		{
			workerThread->join();
		}
	}

	void IntroState::loadAssetsInBg()
	{
		std::cout << "[ASYNC] Menu loading...\n";

		game->menuImageBuffer.clear();
		game->menuUiBuffer.clear();

		std::vector<std::string> buttonsNames = {
			"start", "achievements", "shop", "settings", "back"
		};

		// ?adowanie te?
		for (int i = 1; i <= 6; ++i)
		{
			std::string filename = std::format("assets/textures/menu/bg_{:01}.png", i);
			sf::Image img;
			if (img.loadFromFile(filename))
			{
				game->menuImageBuffer.push_back(std::move(img));
			}
			else
			{
				std::cerr << "[ASYNC ERROR] Cannot find " << filename << '\n';
			}
		}

		// ?adowanie przycisków
		for (const auto& name : buttonsNames) {
			std::string filename = "assets/textures/menu/" + name + ".png";
			sf::Image img;
			if (img.loadFromFile(filename))
			{
				game->menuUiBuffer[name] = std::move(img);
			}
			else
			{
				std::cerr << "[ASYNC ERROR] Cannot find " << filename << '\n';
			}
		}

		std::cout << "[ASYNC] Menu loaded.\n";
		isMenuLoaded = true;

		// ?adowanie konfiguracji JSON
		std::cout << "[ASYNC] Characters config loading...\n";
		std::ifstream configFile("assets/configs/fruits.json");
		if (configFile.is_open())
		{
			try
			{
				configFile >> (game->fruitsConfig);
				isConfigLoaded = true;
				std::cout << "[ASYNC] fruits.json loaded.\n";
			}
			catch (const nlohmann::json::parse_error& e)
			{
				std::cerr << "[ASYNC ERROR] " << e.what() << "\n";
			}
		}
	}

	StateType IntroState::getType() const
	{
		return StateType::Intro;
	}

	void IntroState::handleEvent(const sf::Event& event)
	{
		if (event.is<sf::Event::KeyPressed>())
		{
			auto keyEvent = event.getIf<sf::Event::KeyPressed>();
			if (keyEvent->code == sf::Keyboard::Key::Space || keyEvent->code == sf::Keyboard::Key::Enter)
			{
				// Pozwól pomin?? intro tylko wtedy, gdy zasoby w tle ju? si? za?adowa?y
				if (isMenuLoaded && isConfigLoaded)
				{
					introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
					return;
				}
				else
				{
					std::cout << "[ASYNC] Menu still loading, cannot skip intro yet...\n";
				}
			}
		}
	}

	void IntroState::update(float dt)
	{
		if (dt > 0.1f) dt = 0.1f;

		elapsedTime += dt;

		// Skalowanie na bie??co w razie zmiany rozmiaru okna
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		videoPlayer.fitToView(viewSize);

		while (elapsedTime >= frameDuration)
		{
			elapsedTime -= frameDuration;

			if (!videoPlayer.isDone())
			{
				videoPlayer.update();
			}
			else
			{
				// Wideo dobieg?o ko?ca, przechodzimy do menu (je?li za?adowane)
				if (isMenuLoaded && isConfigLoaded)
				{
					introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
					return;
				}
			}
		}
	}

	void IntroState::render(sf::RenderWindow& window)
	{
		window.draw(videoPlayer.getSprite());
	}
}