// --- DNA.h ---

#pragma once
#include <random>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdint>

namespace game::genetics
{
    
    enum class AiBehavior {
        Charger,    // Rushes directly at the player (Broccoli, Potato)
        Sniper,     // Tries to maintain a specific long distance (Carrot)
        Skirmisher, // Erratic movement, hit-and-run tactics
        Stationary, // Zatrzymuje si? na dystans i nie rusza (Corn)
        Kamikaze    // Biegnie do gracza i wybucha (Garlic)
    };

    struct DNA
    {
        // --- GENOTYPE (The actual genes) ---
        std::string skinKey = "Tomato";

        float speed = 100.0f;
        float maxHp = 100.0f;
        float sizeScale = 1.0f;

        int r = 255, g = 255, b = 255;

        AiBehavior behavior = AiBehavior::Charger;
        std::vector<std::string> abilities;

        // --- ECONOMY / LOOT GENETICS ---
        float dropChance = 1.0f;  // 1.0 = 100% chance
        float baseJuice = 10.0f;  // Base amount of currency dropped

		// for mutations tracking
        bool isMutated = false;

		// for clone tracking
        bool isClone = false;

        // --- PHENOTYPE RESULTS ---
        // Fitness score determines how "successful" this mutant was in the arena
        float fitnessScore = 0.0f;

        // --- EVOLUTIONARY METHODS ---

        // 1. Crossover: Combines this DNA with a partner's DNA to create offspring
        DNA crossover(const DNA& partner, std::mt19937& rng) const
        {
            DNA child;
            std::uniform_int_distribution<int> coinFlip(0, 1);

            // Wygl?d i zachowanie losujemy (50/50 czy po ojcu czy po matce)
            child.skinKey = coinFlip(rng) ? this->skinKey : partner.skinKey;
            child.behavior = coinFlip(rng) ? this->behavior : partner.behavior;

            // Kolor (je?li u?ywamy tint) te? losujemy
            child.r = coinFlip(rng) ? this->r : partner.r;
            child.g = coinFlip(rng) ? this->g : partner.g;
            child.b = coinFlip(rng) ? this->b : partner.b;

            // Statystyki fizyczne U?REDNIAMY (krzy?owanie)
            child.speed = (this->speed + partner.speed) / 2.0f;
            child.maxHp = (this->maxHp + partner.maxHp) / 2.0f;
            child.sizeScale = (this->sizeScale + partner.sizeScale) / 2.0f;
            child.dropChance = (this->dropChance + partner.dropChance) / 2.0f;
            child.baseJuice = (this->baseJuice + partner.baseJuice) / 2.0f;

            // --- PRAWDZIWE ??CZENIE UMIEJ?TNO?CI ---
            child.abilities = this->abilities; // Bierzemy geny rodzica A
            for (const auto& ab : partner.abilities)
            {
                // Je?li dziecko jeszcze nie ma genu rodzica B, ma 50% szans go odziedziczy?
                if (std::find(child.abilities.begin(), child.abilities.end(), ab) == child.abilities.end())
                {
                    if (coinFlip(rng)) {
                        child.abilities.push_back(ab);
                    }
                }
            }

            // Ograniczamy do max 3 umiej?tno?ci
            if (child.abilities.size() > 3) {
                std::shuffle(child.abilities.begin(), child.abilities.end(), rng);
                child.abilities.resize(3);
            }

            child.isMutated = true; // Potomek jest mutantem

            return child;
        }

        // 2. Mutation: Introduces small random anomalies into the child's genes
        void mutate(float mutationRate, std::mt19937& rng)
        {
            std::uniform_real_distribution<float> chance(0.0f, 1.0f);
            bool mutatedThisGeneration = false;

            // Mutate Speed
            if (chance(rng) < mutationRate) {
                std::uniform_real_distribution<float> mod(0.8f, 1.2f);
                speed *= mod(rng);
                speed = std::clamp(speed, 50.0f, 600.0f);
                mutatedThisGeneration = true;
            }

            // Mutate HP
            if (chance(rng) < mutationRate) {
                std::uniform_real_distribution<float> mod(0.8f, 1.3f);
                maxHp *= mod(rng);
                mutatedThisGeneration = true;
            }

            // Mutate Size and Color
            if (chance(rng) < mutationRate) {
                std::uniform_real_distribution<float> mod(0.9f, 1.15f);
                sizeScale *= mod(rng);
                sizeScale = std::clamp(sizeScale, 0.5f, 3.0f);

                std::uniform_int_distribution<int> colorShift(-40, 40);
                r = std::clamp(r + colorShift(rng), 0, 255);
                g = std::clamp(g + colorShift(rng), 0, 255);
                b = std::clamp(b + colorShift(rng), 0, 255);
                mutatedThisGeneration = true;
            }

            // Mutate Juice Amount (Przetrwanie najsilniejszych - oddaj¹ mniej soku!)
            if (chance(rng) < (mutationRate * 0.8f)) {
                std::uniform_real_distribution<float> mod(0.8f, 1.2f);
                baseJuice *= mod(rng);
                baseJuice = std::max(1.0f, baseJuice); // Minimum 1 punkt soku
                mutatedThisGeneration = true;
            }

            // Radical Mutation: Behavior change
            if (chance(rng) < (mutationRate * 0.5f)) {
                std::uniform_int_distribution<int> behDist(0, 2);
                behavior = static_cast<AiBehavior>(behDist(rng));
                mutatedThisGeneration = true;
            }

            //// Radical Mutation: Abilities change
            //if (chance(rng) < (mutationRate * 0.5f)) {
            //    static const std::vector<std::string> possibleAbilities = {
            //        "Shoot",
            //        "Armor",
            //        "SplitOnDeath",
            //        "WindupBruiser",
            //        "PoisonExplosion",
            //        "Dash"
            //    };

            //    std::uniform_int_distribution<size_t> poolDist(0, possibleAbilities.size() - 1);
            //    std::string mutatedAbility = possibleAbilities[poolDist(rng)];

            //    auto it = std::find(abilities.begin(), abilities.end(), mutatedAbility);
            //    if (it == abilities.end()) {
            //        if (abilities.size() < 3) {
            //            abilities.push_back(mutatedAbility);
            //        }
            //        else {
            //            std::uniform_int_distribution<size_t> replaceDist(0, abilities.size() - 1);
            //            abilities[replaceDist(rng)] = mutatedAbility;
            //        }
            //    }
            //    mutatedThisGeneration = true;
            //}

            if (mutatedThisGeneration) {
                isMutated = true;
            }
        }
    };
}