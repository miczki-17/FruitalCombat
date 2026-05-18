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

		// Odpalamy w¹tek pobierania danych w tle
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

		// Czyszczenie buforów
		game->menuImageBuffer.clear();
		game->menuUiBuffer.clear();
		game->characterImageBuffer.clear();
		game->mapImageBuffer.clear();

		// ==========================================
		// 1. £ADOWANIE CONFIGU POSTACI I TEKSTUR
		// ==========================================
		std::cout << "[ASYNC] Characters config loading...\n";
		std::ifstream configFile("assets/configs/fruits.json");
		if (configFile.is_open())
		{
			try {
				configFile >> (game->fruitsConfig);
				isConfigLoaded = true;
				std::cout << "[ASYNC] fruits.json loaded successfully.\n";

				for (auto& [characterKey, characterData] : game->fruitsConfig.items()) {
					if (characterData.contains("texturePath")) {
						std::string path = characterData.value("texturePath", "");
						if (!path.empty()) {
							sf::Image img;
							if (img.loadFromFile(path)) {
								game->characterImageBuffer[characterKey] = std::move(img);
							}
						}
					}
				}
			}
			catch (const nlohmann::json::parse_error& e) {
				std::cerr << "[ASYNC ERROR] fruits.json Parse error: " << e.what() << "\n";
				isConfigLoaded = false;
			}
		}
		else {
			std::cerr << "[ASYNC ERROR] Cannot open fruits.json!\n";
			isConfigLoaded = false;
		}

		// ==========================================
		// 2. £ADOWANIE CONFIGU MAP I TEKSTUR
		// ==========================================
		std::cout << "[ASYNC] Maps config loading...\n";
		std::ifstream mapsFile("assets/configs/maps.json");
		if (mapsFile.is_open())
		{
			try {
				mapsFile >> (game->mapsConfig);
				isMapConfigLoaded = true;
				std::cout << "[ASYNC] maps.json loaded successfully.\n";

				for (auto& [mapKey, mapData] : game->mapsConfig.items()) {
					if (mapData.contains("thumbnailPath")) {
						std::string path = mapData.value("thumbnailPath", "");
						if (!path.empty()) {
							sf::Image img;
							if (img.loadFromFile(path)) {
								game->mapImageBuffer[mapKey] = std::move(img);
								std::cout << "[ASYNC] Loaded map thumbnail for: " << mapKey << "\n";
							}
						}
					}
				}
			}
			catch (const nlohmann::json::parse_error& e) {
				std::cerr << "[ASYNC ERROR] maps.json Parse error: " << e.what() << "\n";
				isMapConfigLoaded = false;
			}
		}
		else {
			std::cerr << "[ASYNC ERROR] Cannot open maps.json!\n";
			isMapConfigLoaded = false;
		}

		//for (const auto& [key, data] : game->mapsConfig.items()) {
		//	std::cout << game->mapsConfig;
		//}

		// ==========================================
		// 3. £ADOWANIE UI I T£A
		// ==========================================
		for (int i = 1; i <= 6; ++i)
		{
			std::string filename = std::format("assets/textures/ui/bg_{:01}.png", i);
			sf::Image img;
			if (img.loadFromFile(filename)) game->menuImageBuffer.push_back(std::move(img));
		}

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
			{"settings_bg", "assets/textures/ui/settings_bg.png" },
			{"select_bg", "assets/textures/ui/select_bg.png"},
			{"hp_icon", "assets/textures/ui/hp_icon.png"},
			{"spd_icon", "assets/textures/ui/spd_icon.png"},
			{"dmg_icon", "assets/textures/ui/dmg_icon.png"},
			{"star_full_icon", "assets/textures/ui/star_full_icon.png"},
			{"star_empty_icon", "assets/textures/ui/star_empty_icon.png"},
			{"resume_btn", "assets/textures/ui/resume_button.png"},
			{"settings_btn", "assets/textures/ui/settings_button_pause.png"},
			{"exit_btn", "assets/textures/ui/exit_button.png",},
			{"yes_btn", "assets/textures/ui/yes_button.png"},
			{"no_btn", "assets/textures/ui/no_button.png"}
		};

		for (const auto& [key, path] : uiPaths) {
			sf::Image img;
			if (img.loadFromFile(path)) game->menuUiBuffer[key] = std::move(img);
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
				if (isMenuLoaded && isConfigLoaded && isMapConfigLoaded) {
					introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
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
				if (isMenuLoaded && isConfigLoaded && isMapConfigLoaded) {
					introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
				}
			}
		}
	}

	void IntroState::render(sf::RenderWindow& window) { window.draw(videoPlayer.getSprite()); }
}