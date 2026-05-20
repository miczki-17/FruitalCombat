#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>

namespace game::core
{
    class ResourceManager
    {
    private:
        std::map<std::string, sf::Texture> textures;
        std::map<std::string, sf::Image> images;
        std::map<std::string, sf::Font> fonts;
        std::map<std::string, sf::SoundBuffer> soundBuffers;

        ResourceManager() = default;

    public:
        // Singleton - gwarancja jednej instancji menedzera w calej grze
        static ResourceManager& get()
        {
            static ResourceManager instance;
            return instance;
        }

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        // --- TEKSTURY ---
        bool loadTexture(const std::string& id, const std::string& filepath)
        {
            sf::Texture tex;
            if (tex.loadFromFile(filepath)) {
                textures[id] = std::move(tex);
                return true;
            }
            std::cerr << "[RES ERROR] Cannot load texture: " << filepath << "\n";
            return false;
        }

        sf::Texture& getTexture(const std::string& id)
        {
            return textures.at(id);
        }

        bool hasTexture(const std::string& id) const { return textures.contains(id); }

        // --- OBRAZY (np. dla masek kolizji) ---
        bool loadImage(const std::string& id, const std::string& filepath)
        {
            sf::Image img;
            if (img.loadFromFile(filepath)) {
                images[id] = std::move(img);
                return true;
            }
            std::cerr << "[RES ERROR] Cannot load image: " << filepath << "\n";
            return false;
        }

        const sf::Image& getImage(const std::string& id) const
        {
            return images.at(id);
        }

        // --- CZCIONKI (SFML 3) ---
        bool loadFont(const std::string& id, const std::string& filepath)
        {
            sf::Font f;
            if (f.openFromFile(filepath)) {
                fonts[id] = std::move(f);
                return true;
            }
            std::cerr << "[RES ERROR] Cannot load font: " << filepath << "\n";
            return false;
        }

        sf::Font& getFont(const std::string& id)
        {
            return fonts.at(id);
        }

        // --- DZWIEKI ---
        bool loadSoundBuffer(const std::string& id, const std::string& filepath)
        {
            sf::SoundBuffer buffer;
            if (buffer.loadFromFile(filepath)) {
                soundBuffers[id] = std::move(buffer);
                return true;
            }
            std::cerr << "[RES ERROR] Cannot load sound: " << filepath << "\n";
            return false;
        }

        sf::SoundBuffer& getSoundBuffer(const std::string& id)
        {
            return soundBuffers.at(id);
        }

        void clearAll()
        {
            textures.clear();
            images.clear();
            fonts.clear();
            soundBuffers.clear();
        }
    };
}