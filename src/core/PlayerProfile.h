// --- PlayerProfile.h ---

#pragma once

namespace game::core
{
    // Definicja typów nawozów
    enum class FertilizerType {
        Regular,
        Medium,
        Best
    };

    struct PlayerProfile final
    {
        // waluta INgame
        int biomassJuice = 0;

        // Poziomy ulepszenia
        int damageLevel = 0;
        int maxHealthLevel = 0;
        int speedLevel = 0;


        // Aktualnie wybrany nawóz
        FertilizerType equippedFertilizer = FertilizerType::Regular;

        // Pomocnicza funkcja do pobierania iloœci aktualnie wybranego nawozu
        int getEquippedFertilizerCount() const {
            switch (equippedFertilizer) {
            case FertilizerType::Regular: return regularFertilizerCount;
            case FertilizerType::Medium:  return mediumFertilizerCount;
            case FertilizerType::Best:    return bestFertilizerCount;
            }
            return 0;
        }

        // Funkcje zwracaj¹ce gotowe mno¿niki ulepsze dla systemów
        float getDamageMultiplier() const { return 1.0f + (damageLevel * 0.15f); }
        float getHealthMultiplier() const { return 1.0f + (maxHealthLevel * 0.20f); }
        float getSpeedMultiplier()  const { return 1.0f + (speedLevel * 0.05f); }

        // Biomasa
        void addJuice(int amount) { if (amount > 0) biomassJuice += amount; }
        bool spendJuice(int amount) {
            if (biomassJuice >= amount) { biomassJuice -= amount; return true; }
            return false;
        }
        // WALUTA
        int coins = 0;

        // Ekwipunek nawozów
        int regularFertilizerCount = 0;
        int mediumFertilizerCount = 0;
        int bestFertilizerCount = 0;

        //coiny
        void addCoins(int amount) { if (amount > 0) coins += amount; }
        bool spendCoins(int amount) {
            if (coins >= amount) { coins -= amount; return true; }
            return false;
        }
    };
}