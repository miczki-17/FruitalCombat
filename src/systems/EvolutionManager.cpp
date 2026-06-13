// --- EvolutionManager.cpp ---

#include "EvolutionManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "vector"

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
        breedingPool.clear(); // Czyszczenie puli

        generateBaseWaveGenomes();

        // --- Aktywujemy timer i flagi dla Fali 1 ---
        waveTimer = 30.0f;          // Fala 1 trwa 30 sekund (krótsza na rozgrzewkê)
        isWaveTimerActive = true;
        spawnTimer = 0.0f;
        spawnInterval = 1.2f;
        pendingShopBreak = false;   // Sklep jest zamkniêty na start
        isShopDelayActive = false;
        shopDelayTimer = 0.0f;

        std::cout << "[EVOLUTION] Wave 1 started properly with timer.\n";
    }

    void EvolutionManager::onEnemyDeath(game::genetics::DNA fallenDNA)
    {
        harvestedDNA.push_back(std::move(fallenDNA));
    }

    // Advanced position validation algorithm using the bitmask + Spiral Search Failsafe
    sf::Vector2f EvolutionManager::getRandomValidPosition(bool aroundPlayer)
    {
        if (!targetPlayer) return { 0.0f, 0.0f };
        auto* targetPlayer_transform = targetPlayer->getComponent<game::components::TransformComponent>();
        if (!targetPlayer_transform) return { 0.0f, 0.0f };

        sf::Vector2f rolledPos;
        bool valid = false;
        int attempts = 0;
        const int maxAttempts = 100;

        // --- 1. G£ÓWNA PÊTLA LOSUJ¥CA ---
        while (!valid && attempts < maxAttempts)
        {
            attempts++;

            if (aroundPlayer && targetPlayer != nullptr)
            {
                // Lekko poszerzony zakres, daje wiêksz¹ szansê trafienia na maskê w w¹skich miejscach
                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
                std::uniform_real_distribution<float> radiusDist(450.0f, 950.0f);

                float randomAngle = angleDist(rng);
                float randomRadius = radiusDist(rng);

                sf::Vector2f spawnOffset(std::cos(randomAngle) * randomRadius, std::sin(randomAngle) * randomRadius);
                rolledPos = targetPlayer_transform->position + spawnOffset;
            }
            else
            {
                std::uniform_real_distribution<float> posDistX(100.0f, static_cast<float>(collisionMask.getSize().x) * mapScale - 100.0f);
                std::uniform_real_distribution<float> posDistY(100.0f, static_cast<float>(collisionMask.getSize().y) * mapScale - 100.0f);
                rolledPos = { posDistX(rng), posDistY(rng) };
            }

            int pixelX = static_cast<int>(rolledPos.x / mapScale);
            int pixelY = static_cast<int>(rolledPos.y / mapScale);

            if (pixelX >= 0 && pixelX < static_cast<int>(collisionMask.getSize().x) &&
                pixelY >= 0 && pixelY < static_cast<int>(collisionMask.getSize().y))
            {
                if (collisionMask.getPixel({ static_cast<unsigned int>(pixelX), static_cast<unsigned int>(pixelY) }) == sf::Color::White)
                {
                    valid = true;
                }
            }
        }

        // --- 2. FAILSAFE: WYSZUKIWANIE SPIRALNE (Spiral Search) ---
        // Aktywuje siê tylko, jeœli wylosowaliœmy 100 razy i ani razu nie trafiliœmy na bia³y pixel.
        if (!valid && targetPlayer != nullptr)
        {
            // Konwertujemy pozycjê gracza na siatkê (maskê kolizji)
            int startX = static_cast<int>(targetPlayer_transform->position.x / mapScale);
            int startY = static_cast<int>(targetPlayer_transform->position.y / mapScale);

            // Zmienne do kontroli poruszania siê po spirali
            int x = 0;
            int y = 0;
            int dx = 0;
            int dy = -1;

            int maxSteps = 10000; // Górny limit pêtli zapobiegaj¹cy freezom (sprawdzi pole 100x100 wokó³ gracza)
            int mapWidth = static_cast<int>(collisionMask.getSize().x);
            int mapHeight = static_cast<int>(collisionMask.getSize().y);

            for (int i = 0; i < maxSteps; i++)
            {
                int checkX = startX + x;
                int checkY = startY + y;

                // Upewniamy siê, czy obecny krok spirali nadal le¿y w granicach mapy
                if (checkX >= 0 && checkX < mapWidth && checkY >= 0 && checkY < mapHeight)
                {
                    // Jeœli trafiliœmy na bia³y pixel, przerywamy wyszukiwanie i zwracamy koordynaty œwiata
                    if (collisionMask.getPixel({ static_cast<unsigned int>(checkX), static_cast<unsigned int>(checkY) }) == sf::Color::White)
                    {
                        // Dodajemy pó³ kafla (mapScale / 2.0f), by wyœrodkowaæ spawn
                        return {
                            (static_cast<float>(checkX) * mapScale) + (mapScale / 2.0f),
                            (static_cast<float>(checkY) * mapScale) + (mapScale / 2.0f)
                        };
                    }
                }

                // Logika "zakrêcania" (tworzenie kwadratowej spirali wokó³ punktu startowego)
                if (x == y || (x < 0 && x == -y) || (x > 0 && x == 1 - y))
                {
                    int temp = dx;
                    dx = -dy;
                    dy = temp;
                }

                x += dx;
                y += dy;
            }

            // --- 3. OSTATECZNY FALLBACK ---
            // Jeœli algorytm jakimœ cudem przeszed³ 10 000 iteracji i nie znalaz³ choæby jednego 
            // bia³ego pixela (np. gracz zglitchowa³ siê pod mapê), spawniemy na graczu, by unikn¹æ crasha.
            return targetPlayer_transform->position;
        }

        return rolledPos;
    }

    void EvolutionManager::update(float dt)
    {
        if (targetPlayer == nullptr || targetPlayer->isDead()) return;

        // 1. ZARZ¥DZANIE CZASEM FALI
        if (isWaveTimerActive)
        {
            waveTimer -= dt;

            int maxSafeEnemies = std::min(100, 15 + (currentWave * 8));

            if (activeEnemies.size() < static_cast<size_t>(maxSafeEnemies))
            {
                spawnTimer -= dt;
                if (spawnTimer <= 0.0f)
                {
                    spawnEnemyFromPool();
                    spawnTimer = spawnInterval;
                }
            }

            if (waveTimer <= 0.0f)
            {
                isWaveTimerActive = false;
                std::cout << "[EVOLUTION] Wave time is over! Waiting for arena clear...\n";
            }
        }
        // --- 2. ODLICZANIE 5 SEKUND DO SKLEPU ---
        else if (isShopDelayActive)
        {
            shopDelayTimer -= dt;
            if (shopDelayTimer <= 0.0f)
            {
                isShopDelayActive = false;
                if (!pendingShopBreak)
                {
                    pendingShopBreak = true; // Koniec czekania
                }
            }
        }
        // 3. CZYSZCZENIE MAPY (Zegar dobi³ do zera, czekamy na eliminacjê niedobitków)
        else
        {
            if (activeEnemies.empty())
            {
                // Zamiast od razu wchodziæ do sklepu, uruchamiamy 5 sekund opóŸnienia!
                isShopDelayActive = true;
                shopDelayTimer = 5.0f;
                std::cout << "[EVOLUTION] Arena cleared! Shop opens in 5 seconds...\n";
            }
        }
    }

    void EvolutionManager::spawnEnemyFromPool()
    {
        if (breedingPool.empty() || targetPlayer == nullptr) return;

        // Bierzemy LOSOWY wzorzec z naszej puli rozrodczej fali (Klonowanie)
        std::uniform_int_distribution<size_t> dist(0, breedingPool.size() - 1);
        game::genetics::DNA nextDNA = breedingPool[dist(rng)];

        sf::Vector2f safeSpawnPos = getRandomValidPosition(true);

        auto enemy = mutantFactory.createMutant(nextDNA, targetPlayer);
        auto* enemy_transform = enemy->getComponent<game::components::TransformComponent>();
        if (!enemy_transform) return;

        enemy_transform->position = safeSpawnPos;

        activeEnemies.push_back(std::move(enemy));
    }

    void EvolutionManager::evolveNextWaveGenomes()
    {
        currentWave++;

        // Reset Timera Fali 
        // W póŸniejszym etapie mo¿esz to uzale¿niæ od numeru fali (np. currentWave * 5s + 30s)
        waveTimer = 55.0f;

        isWaveTimerActive = true;
        spawnTimer = 0.5f;

        isShopDelayActive = false;
        shopDelayTimer = 0.0f;

        // Zwiêkszamy szybkoœæ spawnowania z ka¿d¹ fal¹!
        spawnInterval = std::max(0.2f, 1.2f - (currentWave * 0.05f));

        mutationRate = std::min(0.45f, 0.12f + (currentWave * 0.02f));

        std::cout << "\n==================================================\n";
        std::cout << "[EVOLUTION] Wave " << currentWave << " breeding session started...\n";

        // Czyœcimy nasz¹ "Pule Rozrodcz¹" (Dawne spawnQueue)
        breedingPool.clear();

        // Jeœli to pierwsza fala (lub gracz nie zabi³ NIKOGO), ³adujemy bazowe geny
        if (harvestedDNA.empty()) {
            generateBaseWaveGenomes();
            return;
        }

        // 1. Sortowanie zebranego DNA po Fitness Score
        std::sort(harvestedDNA.begin(), harvestedDNA.end(), [](const game::genetics::DNA& a, const game::genetics::DNA& b) {
            return a.fitnessScore > b.fitnessScore;
            });

        // Bierzemy 50% najlepszych (Elity)
        size_t eliteCount = std::max(static_cast<size_t>(1), harvestedDNA.size() / 2);
        std::vector<game::genetics::DNA> elites(harvestedDNA.begin(), harvestedDNA.begin() + eliteCount);
        harvestedDNA.clear(); // Czyœcimy cmentarzysko na kolejn¹ falê

        // 2. Krzy¿owanie najlepszych (Tworzymy np. 15 wzorców ewolucyjnych)
        int poolSize = 15;

        // --- Matematyka Mno¿ników (Skalowanie Trudnoœci Fali) ---
        // Obliczamy raz i APLIKUJEMY do ka¿dego wzorca
        float waveHpMultiplier = std::pow(1.10f, currentWave); // +10% HP co falê
        float waveSpeedMultiplier = std::pow(1.02f, currentWave); // +2% Speed co falê
        float waveDmgMultiplier = std::pow(1.05f, currentWave); // Obra¿enia pocisków wrogów (jeœli dodasz do DNA)

        for (int i = 0; i < poolSize; ++i)
        {
            std::uniform_int_distribution<size_t> dist(0, elites.size() - 1);
            const auto& parentA = elites[dist(rng)];
            const auto& parentB = elites[dist(rng)];

            game::genetics::DNA childDNA = parentA.crossover(parentB, rng);
            childDNA.mutate(mutationRate, rng);
            applyGeneticRules(childDNA);
            childDNA.fitnessScore = 0.0f;

            // --- APLIKACJA MATEMATYKI FALI DO DZIECKA ---
            // dziecko ma swój "base", a my modyfikujemy go przed wpuszczeniem do puli.
            childDNA.maxHp *= waveHpMultiplier;
            childDNA.speed *= waveSpeedMultiplier;
            childDNA.damageMultiplier *= waveDmgMultiplier;

            // Limitujemy prêdkoœæ maksymaln¹ ¿eby gra nie sta³a siê zepsuta
            childDNA.speed = std::clamp(childDNA.speed, 50.0f, 450.0f);

            breedingPool.push_back(childDNA);
        }

        // 3. Wstrzykiwanie "Œwie¿ej Krwi" (¯eby zapobiec stagnacji genetycznej)
        // Wrzucamy 5 losowych, czystych genotypów z JSON-a do puli rozrodczej
        std::vector<game::genetics::DNA> cleanPool = getCleanGenomesFromConfig();
        if (!cleanPool.empty()) {
            std::uniform_int_distribution<size_t> dist(0, cleanPool.size() - 1);
            for (int i = 0; i < 5; ++i) {
                game::genetics::DNA freshDNA = cleanPool[dist(rng)];

                // Czyste DNA równie¿ musi dostaæ buffy fali!
                freshDNA.maxHp *= waveHpMultiplier;
                freshDNA.speed *= waveSpeedMultiplier;
                freshDNA.speed = std::clamp(freshDNA.speed, 50.0f, 450.0f);

                breedingPool.push_back(freshDNA);
            }
        }

        std::cout << "[EVOLUTION] Wave mathematical multipliers applied. HPx" << waveHpMultiplier << " | SPDx" << waveSpeedMultiplier << "\n";
        std::cout << "[EVOLUTION] Breeding Pool updated with " << breedingPool.size() << " blueprints.\n";
        std::cout << "==================================================\n\n";
    }

    std::vector<game::genetics::DNA> EvolutionManager::getCleanGenomesFromConfig()
    {
        // Pula rzadkich i niebezpiecznych umiejêtnoœci
        std::vector<std::string> extraAbilities = {
            "Dash"
            // ...
        };

        std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
        std::uniform_int_distribution<size_t> abDist(0, extraAbilities.size() - 1);

        std::vector<game::genetics::DNA> pool;
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
            dna.damageMultiplier = 1.0f;

            std::string beh = data.value("behavior", "Charger");
            dna.behavior = strToBehavior(beh);

            if (data.contains("abilities")) {
                for (const auto& ab : data["abilities"]) {
                    dna.abilities.push_back(ab.get<std::string>());
                }
            }

            float waveDmgMultiplier = std::pow(1.05f, currentWave);
            dna.damageMultiplier *= waveDmgMultiplier;

            // --- (Rzadkie Mutacje) ---

            // Szansa roœnie z ka¿d¹ fal¹
            float rareChance = 0.02f + (currentWave * 0.5 * 0.005f);

            if (chanceDist(rng) <= rareChance)
            {
                std::string rareAb = extraAbilities[abDist(rng)];

                // Upewniamy siê, ¿e genotyp jeszcze nie ma tej zdolnoœci
                if (std::find(dna.abilities.begin(), dna.abilities.end(), rareAb) == dna.abilities.end())
                {
                    dna.abilities.push_back(rareAb);

                    // --- WIZUALNE OZNACZENIE ELITY ---
                    dna.sizeScale *= 1.25f; // Jest o 25% wiêkszy od pobratymców
                    dna.maxHp *= 1.50f;     // Ma 50% wiêcej bazowego zdrowia
                    dna.r = 255;            // Zabieramy zielony i niebieski kolor...
                    dna.g = 80;             // ...by nadaæ mu groŸny, agresywny szkar³atno-czerwony odcieñ
                    dna.b = 80;
                    dna.isMutated = true;   // Oznaczamy jako mutant, by wypad³o z niego wiêcej monet
                    

                    std::cout << "[EVOLUTION] Patient Zero created! A " << dna.skinKey << " evolved: " << rareAb << "!\n";
                }
            }

            pool.push_back(dna);
        }
        return pool;
    }

    void EvolutionManager::generateBaseWaveGenomes()
    {
        std::vector<game::genetics::DNA> basePool = getCleanGenomesFromConfig();
        if (basePool.empty()) basePool.push_back(game::genetics::DNA());

        breedingPool.clear();
        for (int i = 0; i < 15; ++i) { // Wystarczy wrzuciæ 15 do bazy
            std::uniform_int_distribution<size_t> dist(0, basePool.size() - 1);
            breedingPool.push_back(basePool[dist(rng)]);
        }
    }

    int EvolutionManager::getCurrentWave() const { return currentWave; }
    bool EvolutionManager::isSpawningActive() const { return spawnTimer <= 0; }

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

            std::vector<std::string> abilitiesToErase = 
            {
                "SplitOnDeath",
                "WindBruisier"
            };

            for (const auto& ability : abilitiesToErase) {
                auto it = std::find(childDNA.abilities.begin(), childDNA.abilities.end(), ability);
                if (it != childDNA.abilities.end()) {
                    childDNA.abilities.erase(it);
                }
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

    float EvolutionManager::getWaveTimeLeft() const
    {
        return std::max(0.0f, waveTimer);
    }

    float EvolutionManager::getShopDelayTimeLeft() const 
    { 
        return std::max(0.0f, shopDelayTimer); 
    }

    bool EvolutionManager::isInShopDelay() const 
    {
        return isShopDelayActive; 
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