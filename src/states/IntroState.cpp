// --- IntroState.h ---

#include "IntroState.h"
#include "../core/ResourceManager.h"
#include "../core/Game.h"
#include <format>
#include "../vendor/nlohmann/json.hpp"
#include <fstream>
#include <iostream>

namespace game::states
{
    using json = nlohmann::json;

    IntroState::IntroState(game::Game* game)
        : State(game)
    {
        std::cout << "[INTRO] Initializing static intro and progress bar...\n";

        initUI();

        // cursor off
        game->getWindow().setMouseCursorVisible(false);

        // Start the background asset loading thread
        workerThread = std::make_unique<std::thread>(&IntroState::loadAssetsInBg, this);
    }

    IntroState::~IntroState()
    {
        if (workerThread && workerThread->joinable()) {
            workerThread->join();
        }
    }

    void IntroState::initUI()
    {
        sf::Vector2f viewSize = game->getWindow().getView().getSize();

        // 1. Intro bg
        std::string imagePath = "assets/textures/ui/bg_1.png";
        if (!introTexture.loadFromFile(imagePath)) {
            std::cerr << "[INTRO ERROR] Failed to load intro image!\n";
        }
        else {
            introTexture.setSmooth(true);
            introSprite.emplace(introTexture);

            introSprite->setScale({
                viewSize.x / static_cast<float>(introTexture.getSize().x),
                viewSize.y / static_cast<float>(introTexture.getSize().y)
                });
        }

        // 2. Pasek Postepu
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
    }

    // --- ASYNC LOADING ---

    void IntroState::loadAssetsInBg()
    {
        std::cout << "[ASYNC] Assets loading started...\n";

        loadProgress = 5;

        loadCharactersConfig();
        loadEnemiesConfig();
        loadMapsConfig();
        loadShopConfig();
        loadFontsAndUI();
        loadAudio();

        loadProgress = 100;
        isFinished = true;
        std::cout << "[ASYNC] All assets safely loaded to RAM.\n";

        if (game->isGameRun)
            game->getStateMachine().changeState(StateType::Lobby);
    }

    void IntroState::loadCharactersConfig()
    {
        std::cout << "[ASYNC] Characters config loading...\n";
        auto& rm = game::core::ResourceManager::get();

        std::ifstream configFile("assets/configs/fruits.json");
        if (!configFile.is_open()) {
            std::cerr << "[ASYNC ERROR] Cannot open fruits.json!\n";
            return;
        }

        try {
            configFile >> (game->fruitsConfig);
            std::cout << "[ASYNC] fruits.json loaded successfully.\n";

            int index = 0;
            int totalItems = game->fruitsConfig.size();

            for (auto& [characterKey, characterData] : game->fruitsConfig.items()) {
                if (characterData.contains("idleTexturePath")) {
                    std::string path = characterData.value("idleTexturePath", "");
                    if (!path.empty()) rm.loadTexture(characterKey + "_idle", path, game::core::AssetGroup::Global);
                }
                if (characterData.contains("initTexturePath")) {
                    std::string path = characterData.value("initTexturePath", "");
                    if (!path.empty()) rm.loadTexture(characterKey + "_start", path, game::core::AssetGroup::Menu);
                }

                index++;
                if (totalItems > 0) loadProgress = 5 + (30 * index / totalItems);
            }
        }
        catch (const json::parse_error& e) {
            std::cerr << "[ASYNC ERROR] fruits.json Parse error: " << e.what() << "\n";
        }
    }

    void IntroState::loadEnemiesConfig()
    {
        std::cout << "[ASYNC] Enemies config loading...\n";
        std::ifstream enemiesFile("assets/configs/enemies.json");
        if (!enemiesFile.is_open()) {
            std::cerr << "[ASYNC ERROR] Cannot open enemies.json!\n";
            return;
        }

        try {
            enemiesFile >> (game->enemiesConfig);
            std::cout << "[ASYNC] enemies.json loaded successfully.\n";
        }
        catch (const json::parse_error& e) {
            std::cerr << "[ASYNC ERROR] enemies.json Parse error: " << e.what() << "\n";
        }

        loadProgress = 35;
    }

    void IntroState::loadMapsConfig()
    {
        std::cout << "[ASYNC] Maps config loading...\n";
        auto& rm = game::core::ResourceManager::get();

        std::ifstream mapsFile("assets/configs/maps.json");
        if (!mapsFile.is_open()) {
            std::cerr << "[ASYNC ERROR] Cannot open maps.json!\n";
            return;
        }

        try {
            mapsFile >> (game->mapsConfig);

            int index = 0;
            int totalMaps = game->mapsConfig.size();

            for (auto& [mapKey, mapData] : game->mapsConfig.items()) {
                if (mapData.contains("thumbnailPath")) {
                    std::string path = mapData.value("thumbnailPath", "");
                    if (!path.empty()) rm.loadTexture(mapKey + "_thumb", path, game::core::AssetGroup::Menu);
                }
                index++;
                if (totalMaps > 0) loadProgress = 35 + (30 * index / totalMaps);
            }
        }
        catch (const json::parse_error& e) {
            std::cerr << "[ASYNC ERROR] maps.json Parse error: " << e.what() << "\n";
        }

        loadProgress = 60;
    }

    void IntroState::loadShopConfig()
    {
        std::cout << "[ASYNC] Shop config loading...\n";
        std::ifstream shopFile("assets/configs/shop.json");

        if (!shopFile.is_open()) {
            std::cerr << "[ASYNC ERROR] Cannot open shop.json!\n";
            return;
        }

        try {
            shopFile >> (game->shopConfig);
            std::cout << "[ASYNC] shop.json loaded successfully.\n";
        }
        catch (const json::parse_error& e) {
            std::cerr << "[ASYNC ERROR] shop.json Parse error: " << e.what() << "\n";
        }
    }

    void IntroState::loadFontsAndUI()
    {
        auto& rm = game::core::ResourceManager::get();

        // 1. GLOWNA CZCIONKA
        rm.loadFont("main_font", "assets/fonts/PixeloidSans.ttf", game::core::AssetGroup::Global);
        if (sf::Font* fontPtr = rm.getFont("main_font")) {
            game->mainFont = *fontPtr;
        }

        std::map<std::string, std::string> enemyIcons = {
            {"enemy_potato_icon", "assets/textures/entities/enemies/potato/idle.png"},
            {"enemy_garlic_icon", "assets/textures/entities/enemies/garlic/idle.png"},
            {"enemy_corn_icon", "assets/textures/entities/enemies/corn/idle.png"},
            {"enemy_carrot_icon", "assets/textures/entities/enemies/carrot/idle.png"},
            {"enemy_broccoli_icon", "assets/textures/entities/enemies/broccoli/idle.png"}
        };

        for (const auto& [key, path] : enemyIcons) {
            rm.loadTexture(key, path, game::core::AssetGroup::Menu);
        }

        loadProgress = 65;

        // 2. ELEMENTY MENU
        for (int i = 1; i <= 1; ++i) {
            std::string filename = std::format("assets/textures/ui/bg_{:01}.png", i);
            rm.loadTexture("bg_" + std::to_string(i), filename, game::core::AssetGroup::Menu);
        }

        // 3. IKONY I ELEMENTY UI
        std::map<std::string, std::string> uiPaths = {
            {"shop", "assets/textures/ui/shop.png"},
            {"achievements", "assets/textures/ui/achievements.png"},
            {"log_platform", "assets/textures/ui/log_platform.png"},
            {"select_bg", "assets/textures/ui/select_bg.png"},
            {"hp_icon", "assets/textures/ui/health_icon.png"},
            {"spd_icon", "assets/textures/ui/speed_icon.png"},
            {"dmg_icon", "assets/textures/ui/damage_icon.png"},
            {"star_full_icon", "assets/textures/ui/star_full_icon.png"},
            {"star_empty_icon", "assets/textures/ui/star_empty_icon.png"},
            { "bestiary_book_bg", "assets/textures/ui/bestiarius_bg.png" },
            {"bestiary_book_bg", "assets/textures/ui/bestiarius_bg.png"},

        };

        std::map<std::string, std::string> uiGlobalPaths = {
            {"empty_button", "assets/textures/ui/empty_button.png"},
            {"settings", "assets/textures/ui/settings_button.png"},
            {"cursor", "assets/textures/ui/cursor.png" },
            {"crosshair", "assets/textures/ui/crosshairV3.png"},
            {"back", "assets/textures/ui/back_button.png"},
            {"left_arrow", "assets/textures/ui/left_arrow.png"},
            {"right_arrow", "assets/textures/ui/right_arrow.png"},
            {"stat_bar_frame", "assets/textures/ui/stat_bar_frame.png"},
            {"stat_bar_fill", "assets/textures/ui/stat_bar_fill.png"},
            {"coin", "assets/textures/entities/drops/juice_coin.png"},
            {"fert_regular", "assets/textures/items/fert_regular.png"},
            {"fert_medium", "assets/textures/items/fert_medium.png"},
            {"fert_best", "assets/textures/items/fert_best.png"},
      
        };

        for (const auto& [key, path] : uiPaths) {
            rm.loadTexture("ui_" + key, path, game::core::AssetGroup::Menu);
        }

        int uiIndex = 0;
        int uiTotal = uiGlobalPaths.size();
        for (const auto& [key, path] : uiGlobalPaths) {
            rm.loadTexture("ui_" + key, path, game::core::AssetGroup::Global);
            uiIndex++;
            loadProgress = 70 + (25 * uiIndex / uiTotal);
        }
    }

    void IntroState::loadAudio()
    {
        auto& rm = game::core::ResourceManager::get();
        rm.loadSound("mouse_click", "assets/audio/ui/click.mp3", game::core::AssetGroup::Global);
        rm.loadMusic("bg_music", "assets/audio/menu/Victory_at_Canopy_Peak.mp3", game::core::AssetGroup::Menu);
    }

    // --- LOGIC ---

    void IntroState::handleEvent(const sf::Event& event)
    {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Space || keyEvent->code == sf::Keyboard::Key::Enter) {
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
        if (introSprite) window.draw(*introSprite);

        window.draw(progressBarBg);
        window.draw(progressBarFill);
    }
}