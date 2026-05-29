// --- ResourceManager.h ---

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>

#include "AssetGroup.h"

namespace game::core
{
    struct TextureResource
    {
        std::shared_ptr<sf::Texture> texture;
        AssetGroup group;
    };

    struct ImageResource
    {
        sf::Image image;
        AssetGroup group;
    };

    struct FontResource
    {
        sf::Font font;
        AssetGroup group;
    };

    struct SoundResource
    {
        sf::SoundBuffer buffer;
        AssetGroup group;
    };

    struct MusicResource
    {
        std::unique_ptr<sf::Music> music;
        AssetGroup group;
    };

    class ResourceManager final
    {
    public:
        static ResourceManager& get();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        // TEXTURES
        bool loadTexture(const std::string& id, const std::string& filepath, AssetGroup group);
        sf::Texture* getTexture(const std::string& id);
        std::shared_ptr<sf::Texture> getTextureShared(const std::string& id);
        bool hasTexture(const std::string& id) const;
        void removeTexture(const std::string& id);

        // IMAGES
        bool loadImage(const std::string& id, const std::string& filepath, AssetGroup group);
        sf::Image* getImage(const std::string& id);

        // FONTS
        bool loadFont(const std::string& id, const std::string& filepath, AssetGroup group);
        sf::Font* getFont(const std::string& id);

        // SOUNDS
        bool loadSound(const std::string& id, const std::string& filepath, AssetGroup group);
        sf::SoundBuffer* getSound(const std::string& id);

        // MUSIC
        bool loadMusic(const std::string& id, const std::string& filepath, AssetGroup group);
        sf::Music* getMusic(const std::string& id);

        // CLEANUP
        void unloadGroup(AssetGroup group);
        void clear();

    private:
        ResourceManager() = default;
        void logError(const std::string& msg) const;

    private:
        std::unordered_map<std::string, TextureResource> textures_;
        std::unordered_map<std::string, ImageResource> images_;
        std::unordered_map<std::string, FontResource> fonts_;
        std::unordered_map<std::string, SoundResource> sounds_;
        std::unordered_map<std::string, MusicResource> music_;
    };
}