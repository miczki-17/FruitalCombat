// --- CombatSystem.h ---

#pragma once
#include <vector>
#include <memory>

namespace game
{
    struct ArenaContext;
    class Game;
}

namespace game::entities
{
    class Entity;
}

namespace game::systems
{
    class EvolutionManager;

    class CombatSystem final
    {
    public:
        CombatSystem(game::Game* game, game::ArenaContext& context, std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef);

        // Przetwarza zebrane dropy soku, przyznaje walut? i ?aduje pasek Ultimate gracza
        void processJuiceCollection(game::entities::Entity* player);

        // Przetwarza wybuchy pocisków, zadaje obra?enia obszarowe, nak?ada HitFlash oraz tworzy strefy kwasu
        void processBulletDamage();

        // Sprawdza stan zdrowia wrogów, obs?uguje ich ?mier?, generuje dropy soku i karmi genami EvolutionManager
        void processEnemyDeaths(game::systems::EvolutionManager& evolutionManager);

    private:
        game::Game* game_;
        game::ArenaContext& context_;
        std::vector<std::unique_ptr<game::entities::Entity>>& enemies_;
    };
}