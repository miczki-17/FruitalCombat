// --- AudioManager.cpp ---

#include "AudioManager.h"
#include "ResourceManager.h"

namespace game::core
{
    AudioManager& AudioManager::get()
    {
        static AudioManager instance;
        return instance;
    }

    void AudioManager::playSound(const std::string& id)
    {
        auto* buffer =
            ResourceManager::get()
            .getSound(id);

        if (!buffer)
        {
            return;
        }

        activeSounds_.emplace_back(*buffer);

        activeSounds_.back().play();
    }

    void AudioManager::playSoundVolume(const std::string& id, float volume)
    {
        auto* buffer =
            ResourceManager::get().getSound(id);

        if (!buffer)
        {
            return;
        }

        activeSounds_.emplace_back(*buffer);

        if (volume < 0.f)
        {
            activeSounds_.back().setVolume(sfxVolume_);
        }
        else
        {
            activeSounds_.back().setVolume(
                std::clamp(volume, 0.f, 100.f));
        }

        activeSounds_.back().play();
    }



    void AudioManager::playMusic(const std::string& id, bool loop)
    {
        auto* music = ResourceManager::get().getMusic(id);

        if (!music)
        {
            return;
        }

        if (currentMusic_)
        {
            currentMusic_->stop();
        }

        currentMusic_ = music;
        currentMusicId_ = id;
        currentMusic_->setLooping(loop);
        currentMusic_->setVolume(musicVolume_);
        currentMusic_->play();
    }

    void AudioManager::playMusicVolume(const std::string& id, bool loop, float volume)
    {
        auto* music = ResourceManager::get().getMusic(id);

        if (!music)
        {
            return;
        }

        if (currentMusic_)
        {
            currentMusic_->stop();
        }

        currentMusic_ = music;
        currentMusicId_ = id;
        currentMusic_->setVolume(volume < 0.f ? musicVolume_ : volume);
        currentMusic_->setLooping(loop);
        currentMusic_->setVolume(musicVolume_);
        currentMusic_->play();
    }


    void AudioManager::stopMusic()
    {
        if (currentMusic_)
        {
            currentMusic_->stop();
        }
        currentMusic_ = nullptr;
        currentMusicId_.clear();
    }

    void AudioManager::stopAllSounds()
    {
        for (auto& sound : activeSounds_)
        {
            sound.stop();
        }

        activeSounds_.clear();
    }

    void AudioManager::pauseMusic()
    {
        if (currentMusic_)
        {
            currentMusic_->pause();
        }
    }

    void AudioManager::setMusicVolume(float volume)
    {
        musicVolume_ = volume;

        if (currentMusic_)
        {
            currentMusic_->setVolume(musicVolume_);
        }
    }

    void AudioManager::setSfxVolume(float volume)
    {
        sfxVolume_ = volume;
    }

    void AudioManager::update()
    {
        activeSounds_.erase(
            std::remove_if(activeSounds_.begin(), activeSounds_.end(), [](const sf::Sound& sound) {
                return sound.getStatus() == sf::SoundSource::Status::Stopped;
                }),
            activeSounds_.end()
                    );
    }

    bool AudioManager::isMusicPlaying(
        const std::string& id) const
    {
        if (!currentMusic_)
        {
            return false;
        }

        return currentMusicId_ == id &&
            currentMusic_->getStatus() ==
            sf::SoundSource::Status::Playing;
    }
}