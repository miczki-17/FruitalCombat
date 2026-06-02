#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <SFML/Graphics.hpp>

namespace game::core
{
    class LocalizationManager final
    {
    public:
        static LocalizationManager& get();

        std::string getText(const std::string& key) const;

        void setLanguage(const std::string& langCode);
        std::string getCurrentLanguageCode() const { return currentLanguage_; }
        std::string getCurrentLanguageName() const;

        void nextLanguage();
        void previousLanguage();

    private:
        LocalizationManager();
        ~LocalizationManager() = default;

        LocalizationManager(const LocalizationManager&) = delete;
        LocalizationManager& operator=(const LocalizationManager&) = delete;

        void initSupportedLanguages();
        bool loadLanguageFile(const std::string& langCode);

        std::string currentLanguage_ = "en"; // Default lang

        std::vector<std::string> supportedLanguages_;
        std::unordered_map<std::string, std::string> languageNames_;

        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> translations_;
    };

    // Helpers
    inline std::string Loc(const std::string& key) { return LocalizationManager::get().getText(key); }

    inline sf::String LocUTF8(const std::string& key) {
        std::string raw = LocalizationManager::get().getText(key);
        return sf::String::fromUtf8(raw.begin(), raw.end());
    }
}