#include "ResourceManager.h"

namespace game::core
{
    ResourceManager& ResourceManager::get()
    {
        static ResourceManager instance;
        return instance;
    }

    void ResourceManager::logError(
        const std::string& msg) const
    {
        std::cerr << "[RESOURCE ERROR] "
            << msg << "\n";
    }

    bool ResourceManager::loadTexture(
        const std::string& id,
        const std::string& filepath)
    {
        sf::Texture texture;

        if (!texture.loadFromFile(filepath))
        {
            logError("Texture load failed: " + filepath);
            return false;
        }

        textures_[id] = std::move(texture);
        return true;
    }

    sf::Texture*
        ResourceManager::getTexture(
            const std::string& id)
    {
        auto it = textures_.find(id);
        if (it == textures_.end())
        {
            return nullptr;
        }
        return &it->second;
    }

    bool ResourceManager::hasTexture(
        const std::string& id) const
    {
        return textures_.contains(id);
    }

    bool ResourceManager::loadImage(
        const std::string& id,
        const std::string& filepath)
    {
        sf::Image image;

        if (!image.loadFromFile(filepath))
        {
            logError("Image load failed: " + filepath);
            return false;
        }

        images_[id] = std::move(image);
        return true;
    }

    sf::Image*
        ResourceManager::getImage(
            const std::string& id) const
    {
        auto it = images_.find(id);
        if (it == images_.end())
        {
            return nullptr;
        }
        return const_cast<sf::Image*>(&it->second);
    }

    bool ResourceManager::loadFont(
        const std::string& id,
        const std::string& filepath)
    {
        sf::Font font;

        if (!font.openFromFile(filepath))
        {
            logError("Font load failed: " + filepath);
            return false;
        }

        fonts_[id] = std::move(font);
        return true;
    }

    sf::Font*
        ResourceManager::getFont(
            const std::string& id)
    {
        auto it = fonts_.find(id);
        if (it == fonts_.end())
        {
            return nullptr;
        }
        return &it->second;
    }

    bool ResourceManager::loadSound(
        const std::string& id,
        const std::string& filepath)
    {
        sf::SoundBuffer buffer;

        if (!buffer.loadFromFile(filepath))
        {
            logError("Sound load failed: " + filepath);
            return false;
        }

        sounds_[id] = std::move(buffer);
        return true;
    }

    sf::SoundBuffer*
        ResourceManager::getSound(
            const std::string& id)
    {
        auto it = sounds_.find(id);
        if (it == sounds_.end())
        {
            return nullptr;
        }
        return &it->second;
    }

    void ResourceManager::clear()
    {
        textures_.clear();
        images_.clear();
        fonts_.clear();
        sounds_.clear();
    }
}