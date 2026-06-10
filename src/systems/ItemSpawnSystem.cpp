#include "ItemSpawnSystem.h"
#include "../entities/Entity.h"
#include "../components/MedkitComponent.h"
#include "../components/ManaPouchComponent.h"
#include "../components/TransformComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/StatsComponent.h"
#include "../components/PickupComponent.h"
#include "../components/PlayerInputComponent.h"
#include "../core/ResourceManager.h"
#include "../core/ArenaContext.h"

namespace game::systems
{
    ItemSpawnSystem::ItemSpawnSystem(
        game::ArenaContext& context,
        std::vector<std::unique_ptr<game::entities::Entity>>& entitiesRef,
        const sf::Image& mask,
        float scale)
        : context_(context),
        entities_(entitiesRef),
        collisionMask_(mask),
        mapScale_(scale)
    {
        // Inicjalizacja generatora losowego przy starcie systemu
        std::random_device rd;
        rng_.seed(rd());
    }

    void ItemSpawnSystem::update(float deltaTime, game::entities::Entity* player)
    {
        // Je?li gracza nie ma w grze, nie ma sensu spawnowa? apteczek
        if (!player || player->isDead()) return;

        int activeMedkitsCount = 0;
        int activeManaPouchesCount = 0;

        // 1. Policz aktywne apteczki na mapie
        for (auto& entity : entities_)
        {
            if (entity->getComponent<game::components::MedkitComponent>())
            {
                activeMedkitsCount++;
            }

            if (entity->getComponent<game::components::ManaPouchComponent>())
            {
                activeManaPouchesCount++;
            }
        }

        auto* playerTransform = player->getComponent<game::components::TransformComponent>();
        auto* playerStats = player->getComponent<game::components::StatsComponent>();

        // 2. Logika podnoszenia apteczek i sakiewek przez gracza
        if (playerTransform && playerStats)
        {
            for (auto& entity : entities_)
            {
                // medkits
                if (auto* medkit = entity->getComponent<game::components::MedkitComponent>())
                {
                    auto* medkitTransform = entity->getComponent<game::components::TransformComponent>();
                    if (medkitTransform)
                    {
                        sf::Vector2f diff = playerTransform->position - medkitTransform->position;
                        float distSq = (diff.x * diff.x) + (diff.y * diff.y);

                        if (distSq < (40.0f * 40.0f)) // Promie? zebrania przedmiotu
                        {
                            playerStats->heal(medkit->healAmount);
                            entity->destroy();
                            activeMedkitsCount--;
                        }
                    }
                }

                // pouches
                if (auto* manaPouch = entity->getComponent<game::components::ManaPouchComponent>())
                {
                    auto* manaPouchTransform = entity->getComponent<game::components::TransformComponent>();
                    if (manaPouchTransform)
                    {
                        sf::Vector2f diff = playerTransform->position - manaPouchTransform->position;
                        float distSq = (diff.x * diff.x) + (diff.y * diff.y);

                        if (distSq < (40.0f * 40.0f)) // Promie? zebrania przedmiotu
                        {
                            playerStats->restoreMana(manaPouch->manaAmount);
                            entity->destroy();
                            activeManaPouchesCount--;
                        }
                    }
                }
            }
        }

        // 3. Logika generowania nowych apteczek
        if (activeMedkitsCount < 2)
        {
            medkitSpawnTimer_ -= deltaTime;
            if (medkitSpawnTimer_ <= 0.0f)
            {
                std::uniform_real_distribution<float> timeDist(10.0f, 20.0f);
                medkitSpawnTimer_ = timeDist(rng_);

                auto newMedkit = std::make_unique<game::entities::Entity>();

                if (auto* transform = newMedkit->getComponent<game::components::TransformComponent>()) {
                    transform->position = getRandomValidPosition();
                }

                std::uniform_real_distribution<float> velX(-150.f, 150.f);
                sf::Vector2f popVelocity = { velX(rng_), -400.f };

                // 1. Cia?o (Fizyka i Grafika) -> TO RYSYJE APTECZK? I J? ANIMUJE
                newMedkit->addComponent(std::make_unique<game::components::PickupComponent>("medkit_item", popVelocity));

                // 2. ?adunek (Leczenie) -> TO DECYDUJE CO SI? STANIE PO ZEBRANIU
                newMedkit->addComponent(std::make_unique<game::components::MedkitComponent>(10.0f));

                entities_.push_back(std::move(newMedkit));
            }
        }

        // 4. Logika generowania nowych mana sakiewek
        if (activeManaPouchesCount < 2)
        {
            manaPouchSpawnTimer_ -= deltaTime;
            if (manaPouchSpawnTimer_ <= 0.0f)
            {
                std::uniform_real_distribution<float> timeDist(10.0f, 20.0f);
                manaPouchSpawnTimer_ = timeDist(rng_);

                auto newManaPouch = std::make_unique<game::entities::Entity>();

                if (auto* transform = newManaPouch->getComponent<game::components::TransformComponent>()) {
                    transform->position = getRandomValidPosition();
                }

                std::uniform_real_distribution<float> velX(-150.f, 150.f);
                sf::Vector2f popVelocity = { velX(rng_), -400.f };

                // 1. Cia?o (Fizyka i Grafika) -> TO RYSYJE APTECZK? I J? ANIMUJE
                newManaPouch->addComponent(std::make_unique<game::components::PickupComponent>("mana_pouch_item", popVelocity));

                // 2. ?adunek (Leczenie) -> TO DECYDUJE CO SI? STANIE PO ZEBRANIU
                newManaPouch->addComponent(std::make_unique<game::components::ManaPouchComponent>(10.0f));

                entities_.push_back(std::move(newManaPouch));
            }
        }

    }

    sf::Vector2f ItemSpawnSystem::getRandomValidPosition()
    {
        sf::Vector2f rolledPos;
        bool valid = false;
        int attempts = 0;
        const int maxAttempts = 100;

        // Dystrybucje skonfigurowane pod wymiary maski mapy
        std::uniform_real_distribution<float> posDistX(100.0f, static_cast<float>(collisionMask_.getSize().x) * mapScale_ - 100.0f);
        std::uniform_real_distribution<float> posDistY(100.0f, static_cast<float>(collisionMask_.getSize().y) * mapScale_ - 100.0f);

        while (!valid && attempts < maxAttempts)
        {
            attempts++;
            rolledPos = { posDistX(rng_), posDistY(rng_) };

            int pixelX = static_cast<int>(rolledPos.x / mapScale_);
            int pixelY = static_cast<int>(rolledPos.y / mapScale_);

            if (pixelX >= 0 && pixelX < static_cast<int>(collisionMask_.getSize().x) &&
                pixelY >= 0 && pixelY < static_cast<int>(collisionMask_.getSize().y))
            {
                if (collisionMask_.getPixel({ static_cast<unsigned int>(pixelX), static_cast<unsigned int>(pixelY) }) == sf::Color::White)
                {
                    valid = true;
                }
            }
        }

        // Failsafe: je?li mapa jest zablokowana, zwró? ?rodek (lub pozycj? gracza)
        if (!valid) return { 500.0f, 500.0f };

        return rolledPos;
    }
}