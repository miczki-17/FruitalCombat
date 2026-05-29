// --- IntroState.cpp ---

#include "IntroState.h"
#include "../core/ResourceManager.h"
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
        float barX = (viewSize.x - barWidth) / 2.f;
        float barY = viewSize.y * 0.9f;

        progressBarBg.setSize({ barWidth, barHeight });
        progressBarBg.setPosition({ barX, barY });
        progressBarBg.setFillColor(sf::Color(50, 50, 50, 200));
        progressBarBg.setOutlineColor(sf::Color(0, 0, 0));
        progressBarBg.setOutlineThickness(2.f);

        progressBarFill.setSize({ 0.f, barHeight });
        progressBarFill.setPosition({ barX, barY });
        progressBarFill.setFillColor(sf::Color(0, 200, 0, 255));

        // 3. Start the background asset loading thread
        workerThread = std::make_unique<std::thread>(&IntroState::loadAssetsInBg, this);
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
        auto& rm = game::core::ResourceManager::get();

        rm.clear();

        loadProgress = 5;

		// 1. LOADING CONFIGURATION AND TEXTURES FOR CHARACTERS
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

                    if (characterData.contains("idleTexturePath")) {
                        std::string path = characterData.value("idleTexturePath", "");
                        if (!path.empty()) rm.loadTexture(characterKey + "_idle", path);
                    }

                    if (characterData.contains("walkTexturePath")) {
                        std::string path = characterData.value("walkTexturePath", "");
                        if (!path.empty()) rm.loadTexture(characterKey + "_walk", path);
                    }

                    
                    if (characterData.contains("initTexturePath")) {
                        std::string path = characterData.value("initTexturePath", "");
                        if (!path.empty()) rm.loadTexture(characterKey + "_start", path);
                    }

                    index++;
                    if (totalItems > 0) loadProgress = 5 + (30 * index / totalItems);
                }
            }
            catch (const nlohmann::json::parse_error& e) {
                std::cerr << "[ASYNC ERROR] fruits.json Parse error: " << e.what() << "\n";
            }
        }

		// 2. LOADING CONFIGURATION AND TEXTURES FOR ENEMIES
        std::cout << "[ASYNC] Enemies config loading...\n";
        std::ifstream enemiesFile("assets/configs/enemies.json");
        if (enemiesFile.is_open())
        {
            try {
                enemiesFile >> (game->enemiesConfig);
                std::cout << "[ASYNC] enemies.json loaded successfully.\n";

                for (auto& [enemyKey, enemyData] : game->enemiesConfig.items()) {
                    if (enemyData.contains("idleTexturePath")) {
                        rm.loadTexture(enemyKey + "_idle", enemyData.value("idleTexturePath", ""));
                    }
                    if (enemyData.contains("walkTexturePath")) {
                        rm.loadTexture(enemyKey + "_walk", enemyData.value("walkTexturePath", ""));
                    }
                }
            }
            catch (const nlohmann::json::parse_error& e) {
                std::cerr << "[ASYNC ERROR] enemies.json Parse error: " << e.what() << "\n";
            }
        }

        loadProgress = 35;

		// 3. LOADING CONFIGURATION AND TEXTURES FOR MAPS
        std::cout << "[ASYNC] Maps config loading...\n";
        std::ifstream mapsFile("assets/configs/maps.json");
        if (mapsFile.is_open())
        {
            try {
                mapsFile >> (game->mapsConfig);

                int index = 0;
                int totalMaps = game->mapsConfig.size();

                for (auto& [mapKey, mapData] : game->mapsConfig.items()) {

                    // arena backgrounds
                    if (mapData.contains("texturePath")) {
                        rm.loadTexture(mapKey + "_map", mapData.value("texturePath", ""));
                    }

                    // miniatures
                    if (mapData.contains("thumbnailPath")) {
                        std::string path = mapData.value("thumbnailPath", "");
                        if (!path.empty()) rm.loadTexture(mapKey + "_thumb", path);
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

		// 4. GLOBAF FONT AND MISC TEXTURES
        rm.loadFont("main_font", "assets/fonts/Minecraftia-Regular.ttf");


        if (sf::Font* fontPtr = rm.getFont("main_font")) {
            game->mainFont = *fontPtr;
        }

        loadProgress = 65;

        // 5. UI ELEMENTS
        for (int i = 1; i <= 1; ++i)
        {
            std::string filename = std::format("assets/textures/ui/bg_{:01}.png", i);
            rm.loadTexture("bg_" + std::to_string(i), filename);
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
            {"select_bg", "assets/textures/ui/select_bg.png"},
            {"hp_icon", "assets/textures/ui/health_icon.png"},
            {"spd_icon", "assets/textures/ui/speed_icon.png"},
            {"dmg_icon", "assets/textures/ui/damage_icon.png"},
            {"star_full_icon", "assets/textures/ui/star_full_icon.png"},
            {"star_empty_icon", "assets/textures/ui/star_empty_icon.png"},
            {"cursor", "assets/textures/ui/cursor.png" },
            {"crosshair", "assets/textures/ui/crosshairV3.png"},
            {"stat_bar_frame", "assets/textures/ui/stat_bar_frame.png"},
            {"stat_bar_fill", "assets/textures/ui/stat_bar_fill.png"}
        };

        int uiIndex = 0;
        int uiTotal = uiPaths.size();
        
        // ResourceManager loading
        for (const auto& [key, path] : uiPaths) {
            rm.loadTexture("ui_" + key, path);

            uiIndex++;
            loadProgress = 70 + (25 * uiIndex / uiTotal);
        }

        // 6. UI SOUNDS (final 5%)
        if (game->uiClickBuffer.loadFromFile("../../../assets/audio/ui/click.mp3")) {
            game->uiClickSound.emplace(game->uiClickBuffer);
        }
        else if (game->uiClickBuffer.loadFromFile("assets/audio/ui/click.mp3")) {
            game->uiClickSound.emplace(game->uiClickBuffer);
        }

        loadProgress = 100;
        isFinished = true;
        std::cout << "[ASYNC] All assets safely loaded to RAM.\n";
    }

    StateType IntroState::getType() const { return StateType::Intro; }

    void IntroState::handleEvent(const sf::Event& event)
    {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyEvent->code == sf::Keyboard::Key::Space || keyEvent->code == sf::Keyboard::Key::Enter)
            {
                if (isFinished) {
                    game->getStateMachine().changeState(StateType::Menu);
                }
            }
        }
    }

    void IntroState::update(float dt)
    {
        elapsedTime += dt;
        int currentProgress = loadProgress.load();

        float maxWidth = progressBarBg.getSize().x;
        float newWidth = maxWidth * (static_cast<float>(currentProgress) / 100.f);
        progressBarFill.setSize({ newWidth, progressBarFill.getSize().y });

        if (isFinished && elapsedTime >= minDisplayTime) {
            game->getStateMachine().changeState(StateType::Menu);
        }
    }

    void IntroState::render(sf::RenderWindow& window)
    {
        if (introSprite.has_value()) {
            window.draw(*introSprite);
        }
        window.draw(progressBarBg);
        window.draw(progressBarFill);
    }
}