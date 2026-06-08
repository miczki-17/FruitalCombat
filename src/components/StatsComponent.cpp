// --- StatsComponent.cpp ---

#include "StatsComponent.h"

#include <algorithm>

namespace game::components
{
    namespace
    {
        constexpr float POISON_TICK_INTERVAL =
            0.5f;

        constexpr float MAX_ULTIMATE_CHARGE =
            100.0f;
    }

    StatsComponent::StatsComponent(
        float maxHealth,
        float attackSpeed,
        float baseMovementSpeed)
        : maxHealth_(maxHealth),
        currentHealth_(maxHealth),
        attackSpeed_(attackSpeed),
        baseMovementSpeed_(
            baseMovementSpeed),
        currentMovementSpeed_(
            baseMovementSpeed)
    {
    }

    void StatsComponent::update(float deltaTime)
    {
        currentMovementSpeed_ = baseMovementSpeed_;

        // Pasywna regeneracja many
        if (currentMana_ < maxMana_) {
            currentMana_ += manaRegenRate_ * deltaTime;
            if (currentMana_ > maxMana_) currentMana_ = maxMana_;
        }

        processEffects(deltaTime);
    }

    void StatsComponent::takeDamage(
        float amount)
    {
        // Obliczamy ostateczne obra?enia:
        // Je?li amount = 20, a damageReduction_ = 0.3 (30% pancerza),
        // to finalDamage = 20 * (1.0 - 0.3) = 20 * 0.7 = 14.
        float finalDamage = amount * (1.0f - damageReduction_);

        currentHealth_ -= finalDamage;

        currentHealth_ =
            std::max(
                0.0f,
                currentHealth_);
    }

    void StatsComponent::addUltCharge(
        float amount)
    {
        ultimateCharge_ +=
            amount *
            ultimateChargeRate_;

        ultimateCharge_ =
            std::min(
                ultimateCharge_,
                maxUltimateCharge_);
    }

    bool StatsComponent::isUltReady()
        const
    {
        return
            ultimateCharge_ >=
            maxUltimateCharge_;
    }

    void StatsComponent::resetUlt()
    {
        ultimateCharge_ =
            0.0f;
    }

    void StatsComponent::addEffect(
        StatusType type,
        float duration,
        float value)
    {
        for (auto& effect : activeEffects_)
        {
            if (effect.type != type)
                continue;

            effect.duration =
                std::max(effect.duration, duration);

            switch (type)
            {
            case StatusType::Poison:
                effect.value =
                    std::max(effect.value, value);
                break;

            case StatusType::Slow:
                // mniejsza wartoœæ = mocniejszy slow
                effect.value =
                    std::min(effect.value, value);
                break;

            case StatusType::SpeedBuff:
                // wiêksza wartoœæ = mocniejszy buff
                effect.value =
                    std::max(effect.value, value);
                break;
            }

            return;
        }

        activeEffects_.push_back({
            type,
            duration,
            value,
            0.0f
            });
    }

    float StatsComponent::getHpPercentage()
        const
    {
        if (
            maxHealth_ <=
            0.0f)
        {
            return 0.0f;
        }

        return
            std::max(
                0.0f,
                currentHealth_) /
            maxHealth_;
    }

    float StatsComponent::getHealth()
        const
    {
        return currentHealth_;
    }

    float StatsComponent::getMaxHealth()
        const
    {
        return maxHealth_;
    }

    float StatsComponent::getAttackSpeed()
        const
    {
        return attackSpeed_;
    }

    int StatsComponent::getBonusProjectiles()
        const
    {
        return
            bonusProjectiles_;
    }

    float StatsComponent::getCurrentSpeed()
        const
    {
        return
            currentMovementSpeed_;
    }

    float StatsComponent::getBaseSpeed()
        const
    {
        return
            baseMovementSpeed_;
    }

    void StatsComponent::setBonusProjectiles(
        int count)
    {
        bonusProjectiles_ =
            count;
    }

    void StatsComponent::processEffects(
        float deltaTime)
    {
        for (
            int index =
            static_cast<int>(
                activeEffects_
                .size()) -
            1;
            index >= 0;
            --index)
        {
            auto& effect =
                activeEffects_[
                    index];

            effect.duration -=
                deltaTime;

            processEffect(
                effect,
                deltaTime);

            if (
                shouldRemoveEffect(
                    effect))
            {
                activeEffects_
                    .erase(
                        activeEffects_
                        .begin() +
                        index);
            }
        }
    }

    void StatsComponent::processEffect(
        ActiveEffect& effect,
        float deltaTime)
    {
        switch (
            effect.type)
        {
        case StatusType::Poison:
            processPoison(
                effect,
                deltaTime);
            break;

        case StatusType::Slow:
            processSlow(
                effect);
            break;

        case StatusType::SpeedBuff:
            processSpeedBuff(
                effect);
            break;
        }
    }

    void StatsComponent::processPoison(
        ActiveEffect& effect,
        float deltaTime)
    {
        effect.tickTimer +=
            deltaTime;

        if (
            effect.tickTimer <
            POISON_TICK_INTERVAL)
        {
            return;
        }

        takeDamage(
            effect.value *
            POISON_TICK_INTERVAL);

        effect.tickTimer =
            0.0f;
    }

    void StatsComponent::processSlow(
        const ActiveEffect& effect)
    {
        currentMovementSpeed_ *=
            effect.value;
    }

    void StatsComponent::processSpeedBuff(
        const ActiveEffect& effect)
    {
        currentMovementSpeed_ *=
            effect.value;
    }

    bool StatsComponent::shouldRemoveEffect(
        const ActiveEffect& effect)
        const
    {
        return
            effect.duration <=
            0.0f;
    }

    void StatsComponent::increaseMaxHealth(
        float amount)
    {
        maxHealth_ += amount;
        currentHealth_ += amount;
    }

    void StatsComponent::heal(
        float amount)
    {
        currentHealth_ = std::min(currentHealth_ + amount, maxHealth_);
    }

    void StatsComponent::multiplyBaseSpeed(
        float multiplier)
    {
        baseMovementSpeed_ *= multiplier;
        currentMovementSpeed_ = baseMovementSpeed_;
    }

    void StatsComponent::multiplyAttackSpeed(
        float multiplier)
    {
        attackSpeed_ *= multiplier;
    }

    void StatsComponent::increaseUltChargeRate(
        float amount)
    {
        ultimateChargeRate_ += amount;
    }


    void StatsComponent::setLastDamageSourceKey(
        const std::string& key)
    {
        lastDamageSourceKey_ = key;
    }

    std::string StatsComponent::getLastDamageSourceKey() const
    {
        return lastDamageSourceKey_;
    }

    // MANA
    void StatsComponent::consumeMana(float amount)
    {
        currentMana_ -= amount;
        if (currentMana_ < 0.0f) currentMana_ = 0.0f;
    }

    void StatsComponent::restoreMana(float amount)
    {
        currentMana_ += amount;
        if (currentMana_ > maxMana_) currentMana_ = maxMana_;
    }

    float StatsComponent::getMana() const { return currentMana_; }
    float StatsComponent::getMaxMana() const { return maxMana_; }

    float StatsComponent::getManaPercentage() const
    {
        if (maxMana_ <= 0.0f) return 0.0f;
        return std::max(0.0f, currentMana_) / maxMana_;
    }

    void StatsComponent::increaseMaxMana(float newMaxMana) { maxMana_ += newMaxMana; }

    bool StatsComponent::hasActiveEffect(StatusType type) const
    {
        for (const auto& effect : activeEffects_)
        {
            if (effect.type == type && effect.duration > 0.0f)
            {
                return true;
            }
        }
        return false;
    }

    void StatsComponent::setDamageReduction(float reduction)
    {
        // Ograniczamy redukcj? do przedzia?u 0.0 (brak) do 1.0 (100% redukcji / niesko?czono??)
        damageReduction_ = std::clamp(reduction, 0.0f, 1.0f);
    }
}