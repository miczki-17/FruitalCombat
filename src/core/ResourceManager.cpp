// --- ResourceManager.cpp ---

#include "ResourceManager.h"

namespace game::core
{
    ResourceManager& ResourceManager::get()
    {
        static ResourceManager instance;
        return instance;
    }

    void ResourceManager::logError(const std::string& msg) const
    {
        std::cerr << "[RESOURCE ERROR] " << msg << "\n";
    }

    // TEXTURES

    bool ResourceManager::loadTexture(const std::string& id, const std::string& filepath, AssetGroup group)
    {
        auto texture = std::make_shared<sf::Texture>();

        if (!texture->loadFromFile(filepath))
        {
            logError("Texture load failed: " + filepath);
            return false;
        }

        textures_[id] = { std::move(texture), group };
        return true;
    }

    sf::Texture* ResourceManager::getTexture(const std::string& id)
    {
        auto it = textures_.find(id);

        if (it == textures_.end())
        {
            return nullptr;
        }

        return it->second.texture.get();
    }

    std::shared_ptr<sf::Texture> ResourceManager::getTextureShared(const std::string& id)
    {
        auto it = textures_.find(id);

        if (it == textures_.end())
        {
            return nullptr;
        }

        return it->second.texture;
    }

    bool ResourceManager::hasTexture(const std::string& id) const
    {
        return textures_.contains(id);
    }

    void ResourceManager::removeTexture(const std::string& id)
    {
        textures_.erase(id);
    }

    // IMAGES

    bool ResourceManager::loadImage(const std::string& id, const std::string& filepath, AssetGroup group)
    {
        sf::Image image;

        if (!image.loadFromFile(filepath))
        {
            logError("Image load failed: " + filepath);
            return false;
        }

        images_[id] = { std::move(image), group };
        return true;
    }

    sf::Image* ResourceManager::getImage(const std::string& id)
    {
        auto it = images_.find(id);

        if (it == images_.end())
        {
            return nullptr;
        }

        return &it->second.image;
    }

    // FONTS

    bool ResourceManager::loadFont(const std::string& id, const std::string& filepath, AssetGroup group)
    {
        sf::Font font;

        if (!font.openFromFile(filepath))
        {
            logError("Font load failed: " + filepath);
            return false;
        }

        fonts_[id] = { std::move(font), group };
        return true;
    }

    sf::Font* ResourceManager::getFont(const std::string& id)
    {
        auto it = fonts_.find(id);

        if (it == fonts_.end())
        {
            return nullptr;
        }

        return &it->second.font;
    }

    // SOUNDS

    bool ResourceManager::loadSound(const std::string& id, const std::string& filepath, AssetGroup group)
    {
        sf::SoundBuffer buffer;

        if (!buffer.loadFromFile(filepath))
        {
            logError("Sound load failed: " + filepath);
            return false;
        }

        sounds_[id] = { std::move(buffer), group };
        return true;
    }

    sf::SoundBuffer* ResourceManager::getSound(const std::string& id)
    {
        auto it = sounds_.find(id);

        if (it == sounds_.end())
        {
            return nullptr;
        }

        return &it->second.buffer;
    }

    // MUSIC

    bool ResourceManager::loadMusic(const std::string& id, const std::string& filepath, AssetGroup group)
    {
        auto music = std::make_unique<sf::Music>();

        if (!music->openFromFile(filepath))
        {
            logError("Music load failed: " + filepath);
            return false;
        }

        music_[id] = { std::move(music), group };
        return true;
    }

    sf::Music* ResourceManager::getMusic(const std::string& id)
    {
        auto it = music_.find(id);

        if (it == music_.end())
        {
            return nullptr;
        }

        return it->second.music.get();
    }

    // UNLOAD GROUP

    void ResourceManager::unloadGroup(AssetGroup group)
    {
        for (auto it = textures_.begin(); it != textures_.end();)
        {
            if (it->second.group == group)
                it = textures_.erase(it);
            else
                ++it;
        }

        for (auto it = images_.begin(); it != images_.end();)
        {
            if (it->second.group == group)
                it = images_.erase(it);
            else
                ++it;
        }

        for (auto it = fonts_.begin(); it != fonts_.end();)
        {
            if (it->second.group == group)
                it = fonts_.erase(it);
            else
                ++it;
        }

        for (auto it = sounds_.begin(); it != sounds_.end();)
        {
            if (it->second.group == group)
                it = sounds_.erase(it);
            else
                ++it;
        }

        for (auto it = music_.begin(); it != music_.end();)
        {
            if (it->second.group == group)
                it = music_.erase(it);
            else
                ++it;
        }
    }

    // CLEAR ALL
    void ResourceManager::clear()
    {
        textures_.clear();
        images_.clear();
        fonts_.clear();
        sounds_.clear();
        music_.clear();
    }
}