// --- GameWorld.cpp ---

#include "GameWorld.h"
#include "Game.h"
#include "ArenaContext.h"
#include "../entities/Entity.h"
#include "../factories/FruitFactory.h"
#include "../factories/MutantFactory.h"
#include "../systems/EvolutionManager.h"
#include "../systems/CollisionSystem.h"
#include "../systems/CombatSystem.h"
#include "../systems/ParticleSystem.h"
#include "../systems/RenderSystem.h"
#include "../systems/MapHazardSystem.h"
#include "../components/StatsComponent.h"
#include "../components/MovementComponent.h"

namespace game::core
{
    GameWorld::GameWorld(game::Game* game, const sf::Image& collisionMask, float mapScale, const sf::Vector2f& startPos)
        : game_(game), collisionMask_(collisionMask), mapScale_(mapScale)
    {
        // 1. Inicjalizacja Gracza
        game::factories::FruitFactory factory(game_->arenaContext, game_->fruitsConfig, collisionMask_, mapScale_, enemies);
        player = factory.createFruit(game_->selectedFruitType);

        if (player) {
            auto* player_transform = player->getComponent<game::components::TransformComponent>(); if (!player_transform) return;
            player_transform->position = startPos;
            lastPlayerPos_ = startPos;
            game_->arenaContext.playerStats = player->getComponent<game::components::StatsComponent>();

            if (auto* moveComp = player->getComponent<game::components::MovementComponent>()) {
                moveComp->setGamePointer(game_);
                // W przysz?o?ci przeka?emy tutaj `friction` z mapData!
            }
        }

        // 2. Inicjalizacja Wrogów i Ewolucji
        mutantFactory = std::make_unique<game::factories::MutantFactory>(game_->arenaContext, game_, collisionMask_, mapScale_);
        evolutionManager = std::make_unique<game::systems::EvolutionManager>(
            *mutantFactory, enemies, player.get(), game_->enemiesConfig, collisionMask_, mapScale_
        );
        evolutionManager->startFirstWave();

        // 3. Powo?anie wszystkich systemów do ?ycia
        collisionSystem = std::make_unique<game::systems::CollisionSystem>(game_->arenaContext, enemies);
        combatSystem = std::make_unique<game::systems::CombatSystem>(game_, game_->arenaContext, enemies);
        particleSystem = std::make_unique<game::systems::ParticleSystem>(game_->arenaContext);
        renderSystem = std::make_unique<game::systems::RenderSystem>(game_->arenaContext);
        particleSystem = std::make_unique<game::systems::ParticleSystem>(game_->arenaContext);
        renderSystem = std::make_unique<game::systems::RenderSystem>(game_->arenaContext);

        // ParticleSystem config from maps json
        const auto& currentMapConfig = game_->mapsConfig[game_->selectedMapKey];

        uint8_t r = currentMapConfig.value("dustR", 200);
        uint8_t g = currentMapConfig.value("dustG", 200);
        uint8_t b = currentMapConfig.value("dustB", 200);
        uint8_t a = currentMapConfig.value("dustA", 150);
        game_->arenaContext.mapDustColor = sf::Color(r, g, b, a);

        mapHazardSystem = std::make_unique<game::systems::MapHazardSystem>(game_->selectedMapKey, currentMapConfig);
    }

    GameWorld::~GameWorld()
    {
        if (game_)
        {
            game_->arenaContext.clear();
        }
    }

    bool GameWorld::requiresShop() const { return evolutionManager->requiresShop(); }
    void GameWorld::resolveShopBreak() { evolutionManager->resolveShopBreak(); }

    void GameWorld::update(float dt)
    {
        if (!player) return;

        // Aktualizacja statystyk (Buffy, DoTy)
        if (auto* stats = player->getComponent<game::components::StatsComponent>()) stats->update(dt);
        for (auto& enemy : enemies) {
            if (auto* stats = enemy->getComponent<game::components::StatsComponent>()) stats->update(dt);
        }


        // Zunifikowane wywo?ania Systemów
        collisionSystem->updateJuiceCollection(player.get(), dt);
        collisionSystem->updateBulletIntersections(dt, collisionMask_, mapScale_);

        combatSystem->processJuiceCollection(player.get());
        combatSystem->processBulletDamage(player.get());
        combatSystem->processAoE(player.get(), dt);

        particleSystem->updateEffects(dt);
        particleSystem->updateParticles(player.get(), dt, lastPlayerPos_, playerDustSpawnTimer_);

        mapHazardSystem->update(dt, game_->arenaContext, player.get());

        combatSystem->processEnemyDeaths(*evolutionManager);
        evolutionManager->update(dt);

        // Aktualizacja logiczna postaci
        player->update(dt);
        for (auto& enemy : enemies) enemy->update(dt);
    }

    void GameWorld::render(sf::RenderWindow& window, std::optional<sf::Sprite>& mapSprite)
    {
        renderSystem->renderWorld(window, mapSprite, player.get(), enemies);
    }
}