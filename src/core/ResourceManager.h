// --- ResourceManager.h ---

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <unordered_map>
#include <string>
#include <memory>
#include <optional>
#include <iostream>

namespace game::core
{
    class ResourceManager final
    {
    public:
        static ResourceManager& get();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        bool loadTexture(const std::string& id, const std::string& filepath);
        bool loadImage(const std::string& id, const std::string& filepath);
        bool loadFont(const std::string& id, const std::string& filepath);
        bool loadSound(const std::string& id, const std::string& filepath);

		// reverse compatibility raw pointer
        sf::Texture* getTexture(const std::string& id);

        // retunr shared pointer
        std::shared_ptr<sf::Texture> getTextureShared(const std::string& id);

        sf::Image* getImage(const std::string& id) const;
        sf::Font* getFont(const std::string& id);
        sf::SoundBuffer* getSound(const std::string& id);

        bool hasTexture(const std::string& id) const;

		// Clean up a specific texture resource
        void removeTexture(const std::string& id);

        void clear();

    private:
        ResourceManager() = default;

    private:
        std::unordered_map<std::string, std::shared_ptr<sf::Texture>> textures_;
        std::unordered_map<std::string, sf::Image> images_;
        std::unordered_map<std::string, sf::Font> fonts_;
        std::unordered_map<std::string, sf::SoundBuffer> sounds_;

        void logError(const std::string& msg) const;
    };
}