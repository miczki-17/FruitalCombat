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
		frameDuration = 1.0f / 30.0f;
		std::cout << "[INTRO] Initializing video intro...\n";

		std::string videoPath = "assets/video/intro/intro.mp4";
		if (!videoPlayer.load(videoPath)) {
			std::cerr << "[INTRO ERROR] Failed to load intro video!\n";
		}

		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		videoPlayer.fitToView(viewSize);

		// Uruchomienie ³adowania asynchronicznego
		workerThread = std::make_unique<std::thread>(&IntroState::loadAssetsInBg, this);

		if (introMusic.openFromFile("assets/audio/intro/intro.mp3")) {
			introMusic.play();
		}
		else {
			std::cerr << "[INTRO ERROR] Cannot load intro music.\n";
		}
	}

	IntroState::~IntroState()
	{
		if (workerThread && workerThread->joinable()) {
			workerThread->join();
		}
	}

	void IntroState::loadAssetsInBg()
	{
		std::cout << "[ASYNC] Assets loading started...\n";

		game->menuImageBuffer.clear();
		game->menuUiBuffer.clear();
		game->characterImageBuffer.clear();

		// 1. NAJPIERW £ADUJEMY CONFIG JSON (bo potrzebujemy œcie¿ek z niego!)
		std::cout << "[ASYNC] Characters config loading...\n";
		std::ifstream configFile("assets/configs/fruits.json");
		if (configFile.is_open())
		{
			try {
				configFile >> (game->fruitsConfig);
				isConfigLoaded = true;
				std::cout << "[ASYNC] fruits.json loaded successfully.\n";
			}
			catch (const nlohmann::json::parse_error& e) {
				std::cerr << "[ASYNC ERROR] JSON Parse error: " << e.what() << "\n";
				isConfigLoaded = false;
			}
		}
		else {
			std::cerr << "[ASYNC ERROR] Cannot open fruits.json!\n";
			isConfigLoaded = false;
		}

		// 2. AUTOMATYCZNE £ADOWANIE TEKSTUR POSTACI Z JSON-a
		if (isConfigLoaded)
		{
			std::cout << "[ASYNC] Reading character paths from JSON...\n";

			// Przechodzimy pêtl¹ przez ka¿dy obiekt w pliku JSON (np. "Apple", "Banana", "Cherry"...)
			for (auto& [characterKey, characterData] : game->fruitsConfig.items())
			{
				// Sprawdzamy, czy dana postaæ ma zdefiniowane pole "texturePath"
				if (characterData.contains("texturePath"))
				{
					std::string path = characterData.value("texturePath", "");

					// Jeœli œcie¿ka nie jest pusta, ³adujemy plik do RAM-u
					if (!path.empty())
					{
						sf::Image img;
						if (img.loadFromFile(path))
						{
							// Zapisujemy w buforze pod kluczem z JSON-a (np. game->characterImageBuffer["Apple"])
							game->characterImageBuffer[characterKey] = std::move(img);
							std::cout << "[ASYNC] Loaded character image for: " << characterKey << " from path: " << path << "\n";
						}
						else
						{
							std::cerr << "[ASYNC ERROR] Failed to load file from texturePath: " << path << "\n";
						}
					}
				}
			}
		}

		// 3. £ADOWANIE T£A MENU (beze zmian)
		for (int i = 1; i <= 6; ++i)
		{
			std::string filename = std::format("assets/textures/ui/bg_{:01}.png", i);
			sf::Image img;
			if (img.loadFromFile(filename)) {
				game->menuImageBuffer.push_back(std::move(img));
			}
			else {
				std::cerr << "[ASYNC ERROR] Cannot find " << filename << '\n';
			}
		}

		// 4. £ADOWANIE ELEMENTÓW INTERFEJSU UI (beze zmian)
		std::map<std::string, std::string> uiPaths = {
			{"start", "assets/textures/ui/start.png"},
			{"settings", "assets/textures/ui/settings_button.png"},
			{"shop", "assets/textures/ui/shop.png"},
			{"achievements", "assets/textures/ui/achievements.png"},
			{"left_arrow", "assets/textures/ui/left_arrow.png"},
			{"right_arrow", "assets/textures/ui/right_arrow.png"},
			{"choose", "assets/textures/ui/choose.png"},
			{"back", "assets/textures/ui/back_button.png"},
			{"log_platform", "assets/textures/ui/log_platform.png"},
			{"settings_bg", "assets/textures/ui/settings_background.png" }
		};

		for (const auto& [key, path] : uiPaths) {
			sf::Image img;
			if (img.loadFromFile(path)) {
				game->menuUiBuffer[key] = std::move(img);
			}
			else {
				std::cerr << "[ASYNC ERROR] Cannot find UI: " << path << '\n';
			}
		}

		isMenuLoaded = true;
		std::cout << "[ASYNC] All assets loaded to RAM safely.\n";
	}

	StateType IntroState::getType() const { return StateType::Intro; }

	void IntroState::handleEvent(const sf::Event& event)
	{
		if (event.is<sf::Event::KeyPressed>())
		{
			auto keyEvent = event.getIf<sf::Event::KeyPressed>();
			if (keyEvent->code == sf::Keyboard::Key::Space || keyEvent->code == sf::Keyboard::Key::Enter)
			{
				if (isMenuLoaded && isConfigLoaded) {
					introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
					return;
				}
			}
		}
	}

	void IntroState::update(float dt)
	{
		if (dt > 0.1f) dt = 0.1f;
		elapsedTime += dt;

		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		videoPlayer.fitToView(viewSize);

		while (elapsedTime >= frameDuration)
		{
			elapsedTime -= frameDuration;
			if (!videoPlayer.isDone()) {
				videoPlayer.update();
			}
			else {
				if (isMenuLoaded && isConfigLoaded) {
					introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
					return;
				}
			}
		}
	}

	void IntroState::render(sf::RenderWindow& window) { window.draw(videoPlayer.getSprite()); }
}