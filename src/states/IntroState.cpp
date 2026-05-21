// --- IntroState.cpp ---


#include "../core/ResourceManager.h"
#include "IntroState.h"
#include "../core/Game.h"
#include <format>
#include "../vendor/nlohmann/json.hpp"
#include <fstream>
#include <iostream>

namespace game::states
{
	IntroState::IntroState(game::Game* game)
		: State(game)
	{
		std::cout << "[INTRO] Initializing static intro and progress bar...\n";

		std::string imagePath = "assets/textures/ui/bg_1.png";
		if (!introTexture.loadFromFile(imagePath)) {
			std::cerr << "[INTRO ERROR] Failed to load intro image!\n";
		}
		else {
			introTexture.setSmooth(true);

			introSprite.emplace(introTexture);
		}

		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		if (introSprite.has_value() && introTexture.getSize().x > 0 && introTexture.getSize().y > 0) {
			introSprite->setScale({
				viewSize.x / static_cast<float>(introTexture.getSize().x),
				viewSize.y / static_cast<float>(introTexture.getSize().y)
				});
		}

		// 2. Configure progress bar
		float barWidth = viewSize.x * 0.8f;
		float barHeight = 20.f;
		float barX = (viewSize.x - barWidth) / 2.f; // Centered
		float barY = viewSize.y * 0.9f;             // Near the bottom

		progressBarBg.setSize({ barWidth, barHeight });
		progressBarBg.setPosition({ barX, barY });
		progressBarBg.setFillColor(sf::Color(50, 50, 50, 200));
		progressBarBg.setOutlineColor(sf::Color(0, 0, 0)); // Black outline
		progressBarBg.setOutlineThickness(2.f);

		progressBarFill.setSize({ 0.f, barHeight }); // Start with 0 width
		progressBarFill.setPosition({ barX, barY });
		progressBarFill.setFillColor(sf::Color(0, 200, 0, 255)); // Green fill

		// 3. Load and play intro music
		//if (introMusic.openFromFile("assets/audio/intro/intro.mp3")) {
		//	introMusic.play();
		//}
		//else {
		//	std::cerr << "[INTRO ERROR] Cannot load intro music.\n";
		//}

		// 4. Start the background asset loading thread
		workerThread = std::make_unique<std::thread>(&IntroState::loadAssetsInBg, this);
	}

	IntroState::~IntroState()
	{
		// Ensure the thread finishes before destroying the state
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
		game->mapImageBuffer.clear();

		loadProgress = 5;

		std::cout << "[ASYNC] Characters config loading...\n";
		std::ifstream configFile("assets/configs/fruits.json");
		if (configFile.is_open())
		{
			try {
				configFile >> (game->fruitsConfig);
				std::cout << "[ASYNC] fruits.json loaded successfully.\n";

				int index = 0;
				int totalItems = game->fruitsConfig.size();

				for (auto& [characterKey, characterData] : game->fruitsConfig.items()) {
					// Wczytywanie tekstury IDLE
					if (characterData.contains("idleTexturePath")) {
						std::string path = characterData.value("idleTexturePath", "");
						if (!path.empty()) {
							// NOWE: ?adujemy dla ECS!
							game::core::ResourceManager::get().loadTexture(characterKey + "_idle", path);

							// STARE: Legacy dla CharacterSelectState
							sf::Image img;
							if (img.loadFromFile(path)) {
								game->characterImageBuffer[characterKey] = std::move(img);
							}
						}
					}

					// Wczytywanie tekstury WALK do ResourceManager
					if (characterData.contains("walkTexturePath")) {
						std::string path = characterData.value("walkTexturePath", "");
						if (!path.empty()) {
							game::core::ResourceManager::get().loadTexture(characterKey + "_walk", path);
						}
					}

					// Wczytywanie tekstury START (Legacy)
					if (characterData.contains("initTexturePath")) {
						std::string path = characterData.value("initTexturePath", "");
						if (!path.empty()) {
							sf::Image img;
							if (img.loadFromFile(path)) {
								game->characterImageBuffer[characterKey + "_start"] = std::move(img);
							}
						}
					}
					index++;
					if (totalItems > 0) loadProgress = 5 + (30 * index / totalItems);
				}
			}
			catch (const nlohmann::json::parse_error& e) {
				std::cerr << "[ASYNC ERROR] fruits.json Parse error: " << e.what() << "\n";
			}
		}


		// ENEMIES CONFIG & TEXTURES
		std::cout << "[ASYNC] Enemies config loading...\n";
		std::ifstream enemiesFile("assets/configs/enemies.json");
		if (enemiesFile.is_open())
		{
			try {
				enemiesFile >> (game->enemiesConfig);
				std::cout << "[ASYNC] enemies.json loaded successfully.\n";

				for (auto& [enemyKey, enemyData] : game->enemiesConfig.items()) {
					if (enemyData.contains("idleTexturePath")) {
						game::core::ResourceManager::get().loadTexture(enemyKey + "_idle", enemyData.value("idleTexturePath", ""));
					}
					if (enemyData.contains("walkTexturePath")) {
						game::core::ResourceManager::get().loadTexture(enemyKey + "_walk", enemyData.value("walkTexturePath", ""));
					}
				}
			}
			catch (const nlohmann::json::parse_error& e) {
				std::cerr << "[ASYNC ERROR] enemies.json Parse error: " << e.what() << "\n";
			}
		}


		loadProgress = 35;

		std::cout << "[ASYNC] Maps config loading...\n";
		std::ifstream mapsFile("assets/configs/maps.json");
		if (mapsFile.is_open())
		{
			try {
				mapsFile >> (game->mapsConfig);

				int index = 0;
				int totalMaps = game->mapsConfig.size();

				for (auto& [mapKey, mapData] : game->mapsConfig.items()) {
					// NOWE: Za?aduj fizyczn? map? z góry do ResourceManagera (koniec z ?adowaniem w PlayingState)
					if (mapData.contains("texturePath")) {
						game::core::ResourceManager::get().loadTexture(mapKey + "_map", mapData.value("texturePath", ""));
					}

					if (mapData.contains("thumbnailPath")) {
						std::string path = mapData.value("thumbnailPath", "");
						if (!path.empty()) {
							sf::Image img;
							if (img.loadFromFile(path)) {
								game->mapImageBuffer[mapKey] = std::move(img);
							}
						}
					}
					index++;
					if (totalMaps > 0) loadProgress = 35 + (30 * index / totalMaps);
				}
			}
			catch (const nlohmann::json::parse_error& e) {
				std::cerr << "[ASYNC ERROR] maps.json Parse error: " << e.what() << "\n";
			}
		}

		loadProgress = 60;

		// ==========================================
		// 3. FONTS
		// ==========================================
		if (!game->mainFont.openFromFile("assets/fonts/Minecraftia-Regular.ttf")) {
			std::cerr << "[ASYNC ERROR] Failed to load main font!\n";
		}
		else {
			std::cout << "[SYSTEM] 'Minecraftia-Regular.ttf' font loaded into RAM!\n";
		}
		loadProgress = 65;

		// ==========================================
		// 3. UI & BACKGROUND TEXTURES (approx. 30% progress)
		// ==========================================
		for (int i = 1; i <= 6; ++i)
		{
			std::string filename = std::format("assets/textures/ui/bg_{:01}.png", i);
			sf::Image img;
			if (img.loadFromFile(filename)) game->menuImageBuffer.push_back(std::move(img));
			loadProgress = 65 + (5 * i / 6); // Progress from 65% to 70%
		}

		std::map<std::string, std::string> uiPaths = {
			{"empty_button", "assets/textures/ui/empty_button.png"},
			{"settings", "assets/textures/ui/settings_button.png"},
			{"shop", "assets/textures/ui/shop.png"},
			{"achievements", "assets/textures/ui/achievements.png"},
			{"left_arrow", "assets/textures/ui/left_arrow.png"},
			{"right_arrow", "assets/textures/ui/right_arrow.png"},
			{"back", "assets/textures/ui/back_button.png"},
			{"log_platform", "assets/textures/ui/log_platform.png"},
			{"settings_bg", "assets/textures/ui/settings_bg.png" },
			{"select_bg", "assets/textures/ui/select_bg.png"},
			{"hp_icon", "assets/textures/ui/hp_icon.png"},
			{"spd_icon", "assets/textures/ui/spd_icon.png"},
			{"dmg_icon", "assets/textures/ui/dmg_icon.png"},
			{"star_full_icon", "assets/textures/ui/star_full_icon.png"},
			{"star_empty_icon", "assets/textures/ui/star_empty_icon.png"},
			{"cursor", "assets/textures/ui/cursor.png" },
			{"crosshair", "assets/textures/ui/crosshairV3.png"}
		};

		int uiIndex = 0;
		int uiTotal = uiPaths.size();
		for (const auto& [key, path] : uiPaths) {
			sf::Image img;
			if (img.loadFromFile(path)) game->menuUiBuffer[key] = std::move(img);

			uiIndex++;
			loadProgress = 70 + (25 * uiIndex / uiTotal); // Progress from 70% to 95%
		}


		// ==========================================
		// 4. UI SOUNDS (final 5%)
		// ==========================================
		if (!game->uiClickBuffer.loadFromFile("../../../assets/audio/ui/click.mp3"))
		{
			std::cerr << "[ASYNC ERROR] Cannot load uiClickBuffer\n";
		}
		else
		{
			// Emplace creates the sound instance directly in the std::optional
			game->uiClickSound.emplace(game->uiClickBuffer);
		}

		std::cout << "[ASYNC] UI sounds loaded successfully.\n";

		// Finalize loading
		loadProgress = 100;
		isFinished = true;
		std::cout << "[ASYNC] All assets safely loaded to RAM.\n";
	}

	StateType IntroState::getType() const { return StateType::Intro; }

	void IntroState::handleEvent(const sf::Event& event)
	{
		// SFML 3: Checking event type using getIf returns std::optional-like pointer
		if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>())
		{
			// Allow skipping the intro by pressing Space or Enter, 
			// but ONLY if the background loading is completely finished.
			if (keyEvent->code == sf::Keyboard::Key::Space || keyEvent->code == sf::Keyboard::Key::Enter)
			{
				if (isFinished) {
					//introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
				}
			}
		}
	}

	void IntroState::update(float dt)
	{
		elapsedTime += dt;

		// Safely retrieve the current progress from the atomic variable
		int currentProgress = loadProgress.load();

		// Update the width of the progress bar fill
		float maxWidth = progressBarBg.getSize().x;
		float newWidth = maxWidth * (static_cast<float>(currentProgress) / 100.f);
		progressBarFill.setSize({ newWidth, progressBarFill.getSize().y });

		// Automatically transition to the Menu state when loading is done 
		// AND the minimum display time has passed (to avoid instant flashes)
		if (isFinished && elapsedTime >= minDisplayTime) {
			//introMusic.stop();
			game->getStateMachine().changeState(StateType::Menu);
		}
	}

	void IntroState::render(sf::RenderWindow& window)
	{
		// Draw the static intro frame and the progress bar layers
		if (introSprite.has_value()) {
			window.draw(*introSprite);
		}
		window.draw(progressBarBg);
		window.draw(progressBarFill);
	}
}