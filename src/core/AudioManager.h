// --- AudioManager.h ---

#pragma once

#include <SFML/Audio.hpp>

#include <vector>
#include <string>
#include <algorithm>

namespace game::core
{
    class AudioManager final
    {
    public:
        static AudioManager& get();

        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;

        void playSound(const std::string& id);
        void playSoundVolume(const std::string& id, float volume = -1.f);
        void playMusic(const std::string& id, bool loop = true);
        void playMusicVolume(const std::string& id, bool loop = true, float volume = -1.f);
        void stopMusic();
        void stopAllSounds();
        void pauseMusic();
        void setMusicVolume(float volume);
        void setSfxVolume(float volume);
        void update();
        
        bool isMusicPlaying(const std::string& id) const;

    private:
        AudioManager() = default;

    private:
        std::vector<sf::Sound> activeSounds_;
        sf::Music* currentMusic_ = nullptr;
        float musicVolume_ = 100.f;
        float sfxVolume_ = 100.f;
        std::string currentMusicId_;

        static constexpr std::size_t MAX_SOUNDS = 32;
    };
}