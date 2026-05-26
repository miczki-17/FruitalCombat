// --- PlayerProfile.h ---

#pragma once

namespace game::core
{
    struct PlayerProfile final
    {
        // G?ówna waluta gry (zast?puje dawne playerJuice w klasie Game)
        int biomassJuice = 0;

        // Poziomy ulepsze? ze sklepu (trzymane w jednym miejscu)
        int damageLevel = 0;
        int maxHealthLevel = 0;
        int speedLevel = 0;

        // Funkcje zwracaj?ce gotowe mno?niki ulepsze? dla systemów
        float getDamageMultiplier() const { return 1.0f + (damageLevel * 0.15f); }
        float getHealthMultiplier() const { return 1.0f + (maxHealthLevel * 0.20f); }
        float getSpeedMultiplier()  const { return 1.0f + (speedLevel * 0.05f); }

        // Bezpieczne operacje na portfelu
        void addJuice(int amount)
        {
            if (amount > 0) biomassJuice += amount;
        }

        bool spendJuice(int amount)
        {
            if (biomassJuice >= amount) {
                biomassJuice -= amount;
                return true;
            }
            return false;
        }
    };
}