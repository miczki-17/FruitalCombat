// --- StatsComponent.h ---

#pragma once

#include "Component.h"

#include <vector>

namespace game::components
{
    enum class StatusType
    {
        Poison,
        Slow,
        SpeedBuff
    };

    struct ActiveEffect
    {
        StatusType type;
        float duration;
        float value;
        float tickTimer = 0.0f;
    };

    class StatsComponent final : public Component
    {
    public:
        StatsComponent(
            float maxHealth,
            float attackSpeed,
            float baseMovementSpeed = 200.0f);

        void update(float deltaTime) override;

        void takeDamage(
            float amount);

        void addEffect(
            StatusType type,
            float duration,
            float value);

        void setDamageReduction(
			float reduction);

        void addUltCharge(
            float amount);

        bool isUltReady() const;

        void resetUlt();

        float getHpPercentage() const;

        float getHealth() const;
        float getMaxHealth() const;

        float getAttackSpeed() const;
        int getBonusProjectiles() const;

        float getCurrentSpeed() const;
        float getBaseSpeed() const;

        void setBonusProjectiles(
            int count);


        // SHOP
        ///////////////////////////////////////////////////////
        void addDamageMultiplier(float pct);
        float getDamageMultiplier() const;

        void addLifesteal(float pct);
        float getLifesteal() const;

        void addArmor(float pct);

        void increaseManaRegen(float amount);

        void increaseMaxHealth(
            float amount);

        void multiplyBaseSpeed(
            float multiplier);

        void multiplyAttackSpeed(
            float multiplier);

        /////////////////////////////////////////////////////

        void heal(
            float amount);

        void increaseUltChargeRate(
            float amount);

        void setLastDamageSourceKey(
            const std::string& key);

        std::string getLastDamageSourceKey() const;

        bool hasActiveEffect(StatusType type) const;


        // --- MANA (JUICE) ---
        void consumeMana(float amount);
        void restoreMana(float amount);
        float getMana() const;
        float getMaxMana() const;
        float getManaPercentage() const;
        void increaseMaxMana(float newMaxMana);

        float getArmor() const { return damageReduction_; }


    private:
        float maxHealth_;
        float currentHealth_;

        float attackSpeed_;
        int bonusProjectiles_ = 0;

        float ultimateCharge_ = 0.0f;
        float maxUltimateCharge_ = 100.0f;
        float ultimateChargeRate_ = 1.0f;

        float baseMovementSpeed_;
        float currentMovementSpeed_;

        float damageMultiplier_ = 1.0f;
        float lifestealPct_ = 0.0f;


        // --- MANA (JUICE) ---
        float maxMana_ = 100.0f;
        float currentMana_ = 100.0f;
        float manaRegenRate_ = 5.0f;


        // --- ARMOR ---
        float damageReduction_ = 0.0f;

        // what gave dmg as last
        std::string lastDamageSourceKey_ = "";

        std::vector<ActiveEffect>
            activeEffects_;

        void processEffects(
            float deltaTime);

        void processEffect(
            ActiveEffect& effect,
            float deltaTime);

        void processPoison(
            ActiveEffect& effect,
            float deltaTime);

        void processSlow(
            const ActiveEffect& effect);

        void processSpeedBuff(
            const ActiveEffect& effect);

        bool shouldRemoveEffect(
            const ActiveEffect& effect) const;
    };
}