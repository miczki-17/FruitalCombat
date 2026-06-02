#include "LocalizationManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "../vendor/nlohmann/json.hpp"

namespace game::core
{
    using json = nlohmann::json;

    LocalizationManager& LocalizationManager::get()
    {
        static LocalizationManager instance;
        return instance;
    }

    LocalizationManager::LocalizationManager()
    {
        initSupportedLanguages();
        if (!supportedLanguages_.empty()) {
            setLanguage(supportedLanguages_[0]);
        }
    }

    void LocalizationManager::initSupportedLanguages()
    {
        // 1. Wczytanie listy dostepnych kodow ("en", "pl", "de", "ru" itd.)
        std::ifstream supportedFile("assets/configs/supported_languages.json");
        if (supportedFile.is_open()) {
            try {
                json j;
                supportedFile >> j;
                for (const auto& lang : j["supported_languages"]) {
                    supportedLanguages_.push_back(lang.get<std::string>());
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[Loc ERROR] Cannot parse supported_languages.json: " << e.what() << "\n";
            }
        }
        else {
            // Awaryjny fallback
            supportedLanguages_ = { "en" };
        }

        // 2. Wczytanie lang names
        std::ifstream namesFile("assets/configs/languages_names.json");
        if (namesFile.is_open()) {
            try {
                json j;
                namesFile >> j;
                for (auto& [key, value] : j.items()) {
                    languageNames_[key] = value.get<std::string>();
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[Loc ERROR] Cannot parse languages_names.json: " << e.what() << "\n";
            }
        }
    }

    void LocalizationManager::setLanguage(const std::string& langCode)
    {
        currentLanguage_ = langCode;

        if (translations_[langCode].empty()) {
            if (!loadLanguageFile(langCode)) {
                std::cerr << "[Loc ERROR] Failed to load language file for: " << langCode << "\n";
            }
        }
    }

    bool LocalizationManager::loadLanguageFile(const std::string& langCode)
    {
        std::string fullPath = "assets/locales/" + langCode + ".json";

        std::ifstream file(fullPath);
        if (!file.is_open()) {
            std::cerr << "[Loc ERROR] Could not open locale file: " << fullPath << "\n";
            return false;
        }

        try {
            json j;
            file >> j;
            for (auto& [key, value] : j.items()) {
                if (value.is_string()) {
                    translations_[langCode][key] = value.get<std::string>();
                }
            }
            std::cout << "[Loc] Successfully loaded: " << fullPath << " (" << translations_[langCode].size() << " keys)\n";
            return true;
        }
        catch (const json::parse_error& e) {
            std::cerr << "[Loc JSON PARSE ERROR] " << e.what() << " in file: " << fullPath << "\n";
            return false;
        }
    }

    std::string LocalizationManager::getText(const std::string& key) const
    {
        auto itLang = translations_.find(currentLanguage_);
        if (itLang != translations_.end()) {
            auto itText = itLang->second.find(key);
            if (itText != itLang->second.end()) {
                return itText->second;
            }
        }
        return "[" + key + "]";
    }

    std::string LocalizationManager::getCurrentLanguageName() const
    {
        auto it = languageNames_.find(currentLanguage_);
        if (it != languageNames_.end()) {
            return it->second;
        }
        return currentLanguage_; // Jesli brak ladnej nazwy, zwraca sam kod np. "en"
    }

    void LocalizationManager::nextLanguage()
    {
        if (supportedLanguages_.empty()) return;
        auto it = std::find(supportedLanguages_.begin(), supportedLanguages_.end(), currentLanguage_);
        if (it != supportedLanguages_.end()) {
            it++;
            if (it == supportedLanguages_.end()) it = supportedLanguages_.begin(); // Petla od poczatku
            setLanguage(*it);
        }
    }

    void LocalizationManager::previousLanguage()
    {
        if (supportedLanguages_.empty()) return;
        auto it = std::find(supportedLanguages_.begin(), supportedLanguages_.end(), currentLanguage_);
        if (it != supportedLanguages_.end()) {
            if (it == supportedLanguages_.begin()) {
                it = supportedLanguages_.end() - 1; // Petla od konca
            }
            else {
                it--;
            }
            setLanguage(*it);
        }
    }
}