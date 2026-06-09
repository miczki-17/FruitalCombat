// --- EvolutionManager.cpp ---

#include "EvolutionManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace game::systems
{
    // Updated Constructor with Mask and Scale assignment
    EvolutionManager::EvolutionManager(game::factories::MutantFactory& factory,
        std::vector<std::unique_ptr<game::entities::Entity>>& enemiesRef,
        game::entities::Entity* player,
        const nlohmann::json& config,
        const sf::Image& mask,
        float scale)
        : mutantFactory(factory), activeEnemies(enemiesRef), targetPlayer(player),
        enemiesConfig(config), collisionMask(mask), mapScale(scale)
    {
        std::random_device rd;
        rng.seed(rd());
    }

    void EvolutionManager::startFirstWave()
    {
        currentWave = 1;
        harvestedDNA.clear();
        spawnQueue.clear();

        generateBaseWaveGenomes();
        spawnTimer = 0.0f;

        std::cout << "[EVOLUTION] Wave 1 Genomes generated. Queue size: " << spawnQueue.size() << "\n";
    }

    void EvolutionManager::update(float dt)
    {
        if (targetPlayer == nullptr || targetPlayer->isDead()) return;

        if (!spawnQueue.empty())
        {
            spawnTimer -= dt;
            if (spawnTimer <= 0.0f)
            {
                spawnNextEnemyFromQueue();
                float adjustedInterval = std::max(0.4f, spawnInterval - (currentWave * 0.05f));
                spawnTimer = adjustedInterval;
            }
        }

        if (activeEnemies.empty() && spawnQueue.empty() && currentWave > 0)
        {
            if (currentWave % 3 == 0 && !pendingShopBreak)
            {
                pendingShopBreak = true;
            }
            else if (!pendingShopBreak)
            {
                evolveNextWaveGenomes();
            }
        }
    }

    void EvolutionManager::onEnemyDeath(const game::genetics::DNA& fallenDNA)
    {
        harvestedDNA.push_back(fallenDNA);
    }

    // Advanced position validation algorithm using the bitmask
    sf::Vector2f EvolutionManager::getRandomValidPosition(bool aroundPlayer)
    {
        auto* targetPlayer_transform = targetPlayer->getComponent<game::components::TransformComponent>(); if (!targetPlayer_transform) return {0.0f, 0.0f};

        sf::Vector2f rolledPos;
        bool valid = false;
        int attempts = 0;
        const int maxAttempts = 100; // Sane limit to prevent infinite loops if map has no white pixels

        while (!valid && attempts < maxAttempts)
        {
            attempts++;

            if (aroundPlayer && targetPlayer != nullptr)
            {
                // Roll orbitally around the player (Staggered spawns logic)
                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
                std::uniform_real_distribution<float> radiusDist(750.0f, 950.0f);

                float randomAngle = angleDist(rng);
                float randomRadius = radiusDist(rng);


                sf::Vector2f spawnOffset(std::cos(randomAngle) * randomRadius, std::sin(randomAngle) * randomRadius);
                rolledPos = targetPlayer_transform->position + spawnOffset;
            }
            else
            {
                // Roll across the entire map bounds (Initial wave 1 distribution logic)
                std::uniform_real_distribution<float> posDistX(100.0f, static_cast<float>(collisionMask.getSize().x) * mapScale - 100.0f);
                std::uniform_real_distribution<float> posDistY(100.0f, static_cast<float>(collisionMask.getSize().y) * mapScale - 100.0f);
                rolledPos = { posDistX(rng), posDistY(rng) };
            }

            // Convert world float coordinates back to mask pixel indices
            int pixelX = static_cast<int>(rolledPos.x / mapScale);
            int pixelY = static_cast<int>(rolledPos.y / mapScale);

            // Bounds check: verify the pixels are actually inside the sf::Image boundaries
            if (pixelX >= 0 && pixelX < static_cast<int>(collisionMask.getSize().x) &&
                pixelY >= 0 && pixelY < static_cast<int>(collisionMask.getSize().y))
            {
                sf::Color maskPixelColor = collisionMask.getPixel({ static_cast<unsigned int>(pixelX), static_cast<unsigned int>(pixelY) });

                // If the pixel is pure white, it's a valid walkable area!
                if (maskPixelColor == sf::Color::White)
                {
                    valid = true;
                }
            }
        }

        // Failsafe: if we completely fail to find a white pixel after 100 tries, 
        // fallback to the player's current known safe position so the game doesn't crash.
        if (!valid && targetPlayer != nullptr)
        {
            return targetPlayer_transform->position;
        }

        return rolledPos;
    }

    void EvolutionManager::spawnNextEnemyFromQueue()
    {
        if (spawnQueue.empty() || targetPlayer == nullptr) return;

        game::genetics::DNA nextDNA = spawnQueue.back();
        spawnQueue.pop_back();

        // Use our new smart mask-aware validation system!
        sf::Vector2f safeSpawnPos = getRandomValidPosition(true);

        auto enemy = mutantFactory.createMutant(nextDNA, targetPlayer);
        auto* enemy_transform = enemy->getComponent<game::components::TransformComponent>(); if (!enemy_transform) return;
        enemy_transform->position = safeSpawnPos;

        activeEnemies.push_back(std::move(enemy));
    }

    void EvolutionManager::evolveNextWaveGenomes()
    {
        currentWave++;
        int newWaveSize = baseWaveSize + (currentWave * 2);
        mutationRate = std::min(0.45f, 0.12f + (currentWave * 0.02f));

        std::cout << "\n==================================================\n";
        std::cout << "[EVOLUTION] Wave " << currentWave << " breeding session started...\n";

        if (harvestedDNA.empty()) {
            generateBaseWaveGenomes();
            return;
        }

        std::sort(harvestedDNA.begin(), harvestedDNA.end(), [](const game::genetics::DNA& a, const game::genetics::DNA& b) {
            return a.fitnessScore > b.fitnessScore;
            });

        size_t eliteCount = std::max(static_cast<size_t>(1), harvestedDNA.size() / 2);
        std::vector<game::genetics::DNA> elites(harvestedDNA.begin(), harvestedDNA.begin() + eliteCount);

        harvestedDNA.clear();

        for (int i = 0; i < newWaveSize; ++i)
        {
            std::uniform_int_distribution<size_t> dist(0, elites.size() - 1);
            const auto& parentA = elites[dist(rng)];
            const auto& parentB = elites[dist(rng)];

            game::genetics::DNA childDNA = parentA.crossover(parentB, rng);
            childDNA.mutate(mutationRate, rng);
            childDNA.fitnessScore = 0.0f;

            spawnQueue.push_back(childDNA);
        }

        spawnTimer = 0.5f;
        std::cout << "[EVOLUTION] " << newWaveSize << " genomes loaded into spawn queue.\n";
        std::cout << "==================================================\n\n";
    }

    void EvolutionManager::generateBaseWaveGenomes()
    {
        std::vector<game::genetics::DNA> basePool;

        if (!enemiesConfig.empty())
        {
            for (auto& [key, data] : enemiesConfig.items())
            {
                bool isSpawnable = data.value("spawnable", true);
                if (!isSpawnable) {
                    continue;
                }

                game::genetics::DNA dna;
                dna.skinKey = key;
                dna.r = data.value("r", 255); dna.g = data.value("g", 255); dna.b = data.value("b", 255);
                dna.speed = data.value("speed", 200.0f);
                dna.maxHp = data.value("maxHp", 100.0f);
                dna.sizeScale = data.value("sizeScale", 1.0f);

                dna.dropChance = data.value("dropChance", 1.0f);
                dna.baseJuice = data.value("baseJuice", 10.0f);

                std::string beh = data.value("behavior", "Charger");
                if (beh == "Sniper")          dna.behavior = game::genetics::AiBehavior::Sniper;
                else if (beh == "Skirmisher") dna.behavior = game::genetics::AiBehavior::Skirmisher;
                else if (beh == "Stationary")      dna.behavior = game::genetics::AiBehavior::Stationary; // <--- DODANE
                else if (beh == "Kamikaze")        dna.behavior = game::genetics::AiBehavior::Kamikaze;
                else                          dna.behavior = game::genetics::AiBehavior::Charger;

                if (data.contains("abilities")) {
                    for (const auto& ab : data["abilities"]) {
                        dna.abilities.push_back(ab.get<std::string>());
                    }
                }
                basePool.push_back(dna);
            }
        }

        if (basePool.empty()) basePool.push_back(game::genetics::DNA());

        for (int i = 0; i < baseWaveSize; ++i)
        {
            std::uniform_int_distribution<size_t> dist(0, basePool.size() - 1);
            spawnQueue.push_back(basePool[dist(rng)]);
        }
    }

    int EvolutionManager::getCurrentWave() const { return currentWave; }
    bool EvolutionManager::isSpawningActive() const { return !spawnQueue.empty(); }

    // spawn splits
    void EvolutionManager::spawnSplits(const game::genetics::DNA& parentDNA, sf::Vector2f position, int count, const std::string& splitSkinKey, float splitScale) // <--- Dodany argument
    {
        if (targetPlayer == nullptr) return;

        for (int i = 0; i < count; ++i)
        {
            game::genetics::DNA childDNA = parentDNA;
            childDNA.isClone = true;

            if (!splitSkinKey.empty()) {
                childDNA.skinKey = splitSkinKey;
            }

            childDNA.maxHp = std::max(5.0f, parentDNA.maxHp * 0.25f);
            childDNA.speed = parentDNA.speed * 1.4f;

            childDNA.sizeScale = parentDNA.sizeScale * splitScale;

            childDNA.behavior = game::genetics::AiBehavior::Charger;

            auto it = std::find(childDNA.abilities.begin(), childDNA.abilities.end(), "SplitOnDeath");
            if (it != childDNA.abilities.end()) {
                childDNA.abilities.erase(it);
            }

            // Tworzymy mutanta
            auto childMutant = mutantFactory.createMutant(childDNA, targetPlayer);
            // Ustawiamy pozycj? na cia?o rodzica + ma?y losowy rozrzut, ?eby nie sta?y w jednym punkcie
            if (auto* transform = childMutant->getComponent<game::components::TransformComponent>()) {
                std::uniform_real_distribution<float> offsetDist(-25.0f, 25.0f);
                transform->position = position + sf::Vector2f(offsetDist(rng), offsetDist(rng));
            }

            activeEnemies.push_back(std::move(childMutant));
        }
    }

    bool EvolutionManager::requiresShop() const
    {
        return pendingShopBreak;
    }

    void EvolutionManager::resolveShopBreak()
    {
        pendingShopBreak = false;
        evolveNextWaveGenomes();
    }
}