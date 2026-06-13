// --- EvolutionManager.h ---

#pragma once
#include <vector>
#include <memory>
#include <random>
#include "../genetics/DNA.h"
#include "../entities/Entity.h"
#include "../factories/MutantFactory.h"
#include "../vendor/nlohmann/json.hpp"
#include <SFML/Graphics/Image.hpp> // Required for sf::Image

namespace game::systems
{
    class EvolutionManager
    {
    private:
        game::factories::MutantFactory& mutantFactory;
        std::vector<std::unique_ptr<game::entities::Entity>>& activeEnemies;
        game::entities::Entity* targetPlayer;
        const nlohmann::json& enemiesConfig;

        // --- MASK VALIDATION DEPENDENCIES ---
        const sf::Image& collisionMask;
        float mapScale;

        // shop request
        bool pendingShopBreak = false;

        int currentWave = 0;
        int baseWaveSize = 3;
        float mutationRate = 0.15f;

        std::vector<game::genetics::DNA> harvestedDNA;
        std::vector<game::genetics::DNA> breedingPool;
        float spawnTimer = 0.0f;
        float spawnInterval = 1.2f;

        std::mt19937 rng;

        // wave timers
        bool isWaveTimerActive = false;
        float waveTimer = 0.0f;

        // --- DELAY DO SKLEPU ---
        bool isShopDelayActive = false;
        float shopDelayTimer = 0.0f;

        void evolveNextWaveGenomes();
        void generateBaseWaveGenomes();
        void spawnEnemyFromPool();

        // --- HELPER METHOD FOR VALID POSITION HUNTING ---
        // Uses a loop to roll coordinates until a walkable white pixel on the mask is found
        sf::Vector2f getRandomValidPosition(bool aroundPlayer);
        std::vector<game::genetics::DNA> getCleanGenomesFromConfig();

        void applyGeneticRules(game::genetics::DNA& dna);
        std::string behaviorToStr(game::genetics::AiBehavior b) const;
        game::genetics::AiBehavior strToBehavior(const std::string& s) const;

    public:
        EvolutionManager(game::factories::MutantFactory& factory,
            std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef,
            game::entities::Entity* player,
            const nlohmann::json& config,
            const sf::Image& mask, // Added mask
            float scale);          // Added scale

        void startFirstWave();
        void update(float dt);
        void onEnemyDeath(game::genetics::DNA fallenDNA);
        int getCurrentWave() const;
        float getWaveTimeLeft() const;
        float getShopDelayTimeLeft() const;
        bool isInShopDelay() const;
        bool isSpawningActive() const;

        // death split
        void spawnSplits(const game::genetics::DNA& parentDNA, sf::Vector2f position, int count, const std::string& splitSkinKey, float splitScale);

        // shop request
        bool requiresShop() const;
        void resolveShopBreak();
    };
}