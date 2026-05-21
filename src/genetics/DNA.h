#pragma once
#include <random>
#include <algorithm>
#include <SFML/Graphics/Color.hpp>

namespace game::genetics
{
    // Determines how the AI will move around the player
    enum class AiBehavior {
        Charger,    // Rushes directly at the player
        Sniper,     // Tries to maintain a specific long distance
        Skirmisher  // Erratic movement, hit-and-run tactics
    };

    

    struct DNA
    {
        // --- GENOTYPE (The actual genes) ---
        std::string skinKey = "Tomato";

        float speed = 200.0f;
        float maxHp = 100.0f;
        float sizeScale = 1.0f;

        int r = 255, g = 255, b = 255;

        AiBehavior behavior = AiBehavior::Charger;

        std::vector<std::string> abilities;

        // --- PHENOTYPE RESULTS ---
        // Fitness score determines how "successful" this mutant was in the arena
        float fitnessScore = 0.0f;

        // --- EVOLUTIONARY METHODS ---

        // 1. Crossover: Combines this DNA with a partner's DNA to create offspring
        DNA crossover(const DNA& partner, std::mt19937& rng) const
        {
            DNA child;
            std::uniform_int_distribution<int> coinFlip(0, 1);

            child.skinKey = coinFlip(rng) ? this->skinKey : partner.skinKey; // FENOTYPE
            child.speed = coinFlip(rng) ? this->speed : partner.speed;
            child.maxHp = coinFlip(rng) ? this->maxHp : partner.maxHp;
            child.sizeScale = coinFlip(rng) ? this->sizeScale : partner.sizeScale;
            child.r = coinFlip(rng) ? this->r : partner.r;
            child.g = coinFlip(rng) ? this->g : partner.g;
            child.b = coinFlip(rng) ? this->b : partner.b;
            child.behavior = coinFlip(rng) ? this->behavior : partner.behavior;
            child.abilities = coinFlip(rng) ? this->abilities : partner.abilities;

            return child;
        }

        // 2. Mutation: Introduces small random anomalies into the child's genes
        void mutate(float mutationRate, std::mt19937& rng)
        {
            std::uniform_real_distribution<float> chance(0.0f, 1.0f);

            // Mutate Speed (up to 20% change)
            if (chance(rng) < mutationRate) {
                std::uniform_real_distribution<float> mod(0.8f, 1.2f);
                speed *= mod(rng);
                speed = std::clamp(speed, 50.0f, 600.0f); // Keep within sane limits
            }

            // Mutate HP
            if (chance(rng) < mutationRate) {
                std::uniform_real_distribution<float> mod(0.8f, 1.3f);
                maxHp *= mod(rng);
            }

            // Mutate Size and Color slightly
            if (chance(rng) < mutationRate) {
                std::uniform_real_distribution<float> mod(0.9f, 1.15f);
                sizeScale *= mod(rng);
                sizeScale = std::clamp(sizeScale, 0.5f, 3.0f);

                // Color mutation shifts hue slightly
                std::uniform_int_distribution<int> colorShift(-40, 40);
                r = std::clamp(r + colorShift(rng), 0, 255);
                g = std::clamp(g + colorShift(rng), 0, 255);
                b = std::clamp(b + colorShift(rng), 0, 255);
            }

            // Radical Mutation: Behavior change
            if (chance(rng) < (mutationRate * 0.5f)) { // Harder to mutate behavior
                std::uniform_int_distribution<int> behDist(0, 2);
                behavior = static_cast<AiBehavior>(behDist(rng));
            }

            // Radical Mutation: Abilities change (Wektory Stringów)
            if (chance(rng) < (mutationRate * 0.5f)) {
                // Pula dost?pnych w grze umiej?tno?ci dla mutantów
                static const std::vector<std::string> possibleAbilities = {
                    "AcidSquirt", "Shotgun", "Dash", "RindRoll"
                };

                std::uniform_int_distribution<size_t> poolDist(0, possibleAbilities.size() - 1);
                std::string mutatedAbility = possibleAbilities[poolDist(rng)];

                // Sprawdzamy, czy mutant ju? przypadkiem nie ma tej umiej?tno?ci
                auto it = std::find(abilities.begin(), abilities.end(), mutatedAbility);
                if (it == abilities.end()) {

                    // Mutacja! Je?li ma mniej ni? 3 umiej?tno?ci, po prostu zyskuje now?.
                    if (abilities.size() < 3) {
                        abilities.push_back(mutatedAbility);
                    }
                    // Je?li ma ju? 3, jedna z nich mutuje (zamienia si?) w now?
                    else {
                        std::uniform_int_distribution<size_t> replaceDist(0, abilities.size() - 1);
                        abilities[replaceDist(rng)] = mutatedAbility;
                    }
                }
            }
        }
    };
}