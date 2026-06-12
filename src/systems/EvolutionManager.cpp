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

        int maxSafeEnemies = 90; // Limit dla bezpiecze?stwa CPU i czytelno?ci gry

        float multiplier = 7.78;

        // Funkcja ro?nie szybko na pocz?tku, a potem bardzo powoli
        int calculatedSize = baseWaveSize + static_cast<int>(multiplier * (std::sqrt(currentWave) - 1.0f));

        // Wybieramy mniejsz? warto?? (Zabezpieczenie przed Armagedonem)
        int newWaveSize = std::min(maxSafeEnemies, calculatedSize);
        mutationRate = std::min(0.45f, 0.12f + (currentWave * 0.02f));

        std::cout << "\n==================================================\n";
        std::cout << "[EVOLUTION] Wave " << currentWave << " breeding session started...\n";

        if (harvestedDNA.empty()) {
            generateBaseWaveGenomes();
            return;
        }

        // 1. Sortowanie po Fitness Score
        std::sort(harvestedDNA.begin(), harvestedDNA.end(), [](const game::genetics::DNA& a, const game::genetics::DNA& b) {
            return a.fitnessScore > b.fitnessScore;
            });

        size_t eliteCount = std::max(static_cast<size_t>(1), harvestedDNA.size() / 2);
        std::vector<game::genetics::DNA> elites(harvestedDNA.begin(), harvestedDNA.begin() + eliteCount);
        harvestedDNA.clear();

        // --- ZAPEWNIENIE RÓ?NORODNO?CI ---
        int randomSpawns = std::max(1, static_cast<int>(newWaveSize * 0.25f)); // 25% fali to czy?ci, nowi wrogowie
        int bredSpawns = newWaveSize - randomSpawns;

        // 2. Krzy?owanie najlepszych (75% fali)
        for (int i = 0; i < bredSpawns; ++i)
        {
            std::uniform_int_distribution<size_t> dist(0, elites.size() - 1);
            const auto& parentA = elites[dist(rng)];
            const auto& parentB = elites[dist(rng)];

            game::genetics::DNA childDNA = parentA.crossover(parentB, rng);
            childDNA.mutate(mutationRate, rng);
            applyGeneticRules(childDNA); // Oczyszczamy DNA naszym interpreterem z JSON-a
            childDNA.fitnessScore = 0.0f;

            spawnQueue.push_back(childDNA);
        }

        // --- Mno?niki wyk?adnicze ---
        // Wyliczane RAZ na ca?? fal?. 
        // HP ro?nie o 4% co fal? (pot?gowanie)
        float waveHpMultiplier = std::pow(1.04f, currentWave);
        // Pr?dko?? ro?nie tylko o 1.2%
        float waveSpeedMultiplier = std::pow(1.012f, currentWave);



        // wstrzykniecie nowych 25 % czystych genow z configu
        std::vector<game::genetics::DNA> basePool;
        for (auto& [key, data] : enemiesConfig.items()) {
            if (key == "geneticRules") continue;
            if (!data.value("spawnable", true)) continue;

            game::genetics::DNA dna;
            dna.skinKey = key;
            dna.r = data.value("r", 255); dna.g = data.value("g", 255); dna.b = data.value("b", 255);
            dna.speed = data.value("speed", 200.0f);
            dna.maxHp = data.value("maxHp", 100.0f);
            dna.sizeScale = data.value("sizeScale", 1.0f);
            dna.baseJuice = data.value("baseJuice", 4.0f);
            dna.dropChance = data.value("dropChance", 1.0f);

            std::string beh = data.value("behavior", "Charger");
            if (beh == "Sniper")          dna.behavior = game::genetics::AiBehavior::Sniper;
            else if (beh == "Skirmisher") dna.behavior = game::genetics::AiBehavior::Skirmisher;
            else if (beh == "Stationary") dna.behavior = game::genetics::AiBehavior::Stationary;
            else if (beh == "Kamikaze")   dna.behavior = game::genetics::AiBehavior::Kamikaze;
            else                          dna.behavior = game::genetics::AiBehavior::Charger;

            if (data.contains("abilities")) {
                for (const auto& ab : data["abilities"]) {
                    dna.abilities.push_back(ab.get<std::string>());
                }
            }
            basePool.push_back(dna);
        }

        if (!basePool.empty()) {
            std::uniform_int_distribution<size_t> poolDist(0, basePool.size() - 1);
            for (int i = 0; i < randomSpawns; ++i) {
                spawnQueue.push_back(basePool[poolDist(rng)]);
            }
        }

        // 4. Tasujemy kolejk?, ?eby wrogowie wychodzili wymieszani (inaczej na ko?cu fali by?yby same randomy)
        std::shuffle(spawnQueue.begin(), spawnQueue.end(), rng);

        spawnTimer = 0.5f;
        std::cout << "[EVOLUTION] " << bredSpawns << " bred and " << randomSpawns << " random genomes queued.\n";
        std::cout << "==================================================\n\n";
    }

    void EvolutionManager::generateBaseWaveGenomes()
    {
        std::vector<game::genetics::DNA> basePool;

        if (!enemiesConfig.empty())
        {
            for (auto& [key, data] : enemiesConfig.items())
            {
				// pomijamy klucz geneticRules, bo to nie jest definicja przeciwnika
                if (key == "geneticRules") continue;

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
                dna.baseJuice = data.value("baseJuice", 4.0f);

                std::string beh = data.value("behavior", "Charger");
                if (beh == "Sniper")          dna.behavior = game::genetics::AiBehavior::Sniper;
                else if (beh == "Skirmisher") dna.behavior = game::genetics::AiBehavior::Skirmisher;
                else if (beh == "Stationary")      dna.behavior = game::genetics::AiBehavior::Stationary;
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
            childDNA.speed = parentDNA.speed * 1.6f;

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
                std::uniform_real_distribution<float> offsetDist(-37.0f, 37.0f);
                transform->position = position + sf::Vector2f(offsetDist(rng), offsetDist(rng));
            }

            childDNA.isMutated = false;

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



    std::string EvolutionManager::behaviorToStr(game::genetics::AiBehavior b) const {
        switch (b) {
        case game::genetics::AiBehavior::Sniper: return "Sniper";
        case game::genetics::AiBehavior::Skirmisher: return "Skirmisher";
        case game::genetics::AiBehavior::Stationary: return "Stationary";
        case game::genetics::AiBehavior::Kamikaze: return "Kamikaze";
        default: return "Charger";
        }
    }

    game::genetics::AiBehavior EvolutionManager::strToBehavior(const std::string& s) const {
        if (s == "Sniper") return game::genetics::AiBehavior::Sniper;
        if (s == "Skirmisher") return game::genetics::AiBehavior::Skirmisher;
        if (s == "Stationary") return game::genetics::AiBehavior::Stationary;
        if (s == "Kamikaze") return game::genetics::AiBehavior::Kamikaze;
        return game::genetics::AiBehavior::Charger;
    }

    void EvolutionManager::applyGeneticRules(game::genetics::DNA& dna)
    {
        if (!enemiesConfig.contains("geneticRules")) return;

        const auto& rules = enemiesConfig["geneticRules"];
        std::string currentBehaviorStr = behaviorToStr(dna.behavior);

        for (const auto& rule : rules)
        {
            bool conditionMet = false;

            // 1. if_behavior_in
            if (rule.contains("if_behavior_in")) {
                for (const auto& b : rule["if_behavior_in"]) {
                    if (b.get<std::string>() == currentBehaviorStr) {
                        conditionMet = true;
                        break;
                    }
                }
            }

            // 2. if_ability
            if (rule.contains("if_ability")) {
                std::string reqAb = rule["if_ability"].get<std::string>();
                if (std::find(dna.abilities.begin(), dna.abilities.end(), reqAb) != dna.abilities.end()) {
                    conditionMet = true;
                }
            }

            // 3. Wykonanie akcji
            if (conditionMet)
            {
                // Akcja A: remove_abilities
                if (rule.contains("remove_abilities")) {
                    for (const auto& abToRemove : rule["remove_abilities"]) {
                        auto it = std::find(dna.abilities.begin(), dna.abilities.end(), abToRemove.get<std::string>());
                        if (it != dna.abilities.end()) {
                            dna.abilities.erase(it);
                        }
                    }
                }

                // Akcja B: force_ability (dodaje, je?li jeszcze nie ma)
                if (rule.contains("force_ability")) {
                    std::string forceAb = rule["force_ability"].get<std::string>();
                    if (std::find(dna.abilities.begin(), dna.abilities.end(), forceAb) == dna.abilities.end()) {
                        dna.abilities.push_back(forceAb);
                    }
                }

                // Akcja C: force_behavior (nadpisuje sztuczn? inteligencj?)
                if (rule.contains("force_behavior")) {
                    dna.behavior = strToBehavior(rule["force_behavior"].get<std::string>());
                    currentBehaviorStr = behaviorToStr(dna.behavior); // aktualizacja lokalnej zmiennej
                }
            }
        }
    }
}