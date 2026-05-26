// --- AbilityComponent.cpp ---

#include "AbilityComponent.h"
#include "../entities/Entity.h"

namespace game::components
{
    void AbilityComponent::setWeapon(
        std::unique_ptr<Ability> weapon)
    {
        weapon_ = std::move(weapon);
    }

    void AbilityComponent::setSkill(
        std::unique_ptr<Ability> skill)
    {
        skill_ = std::move(skill);
    }

    void AbilityComponent::setUltimate(
        std::unique_ptr<Ability> ultimate)
    {
        ultimate_ = std::move(ultimate);
    }

    void AbilityComponent::update(
        float deltaTime)
    {
        if (weapon_)
        {
            weapon_->update(deltaTime);
        }

        if (skill_)
        {
            skill_->update(deltaTime);
        }

        if (ultimate_)
        {
            ultimate_->update(deltaTime);
        }
    }

    void AbilityComponent::useWeapon(
        sf::Vector2f targetWorldPos)
    {
        executeAbility(
            weapon_,
            targetWorldPos);
    }

    void AbilityComponent::useSkill(
        sf::Vector2f targetWorldPos)
    {
        executeAbility(
            skill_,
            targetWorldPos);
    }

    void AbilityComponent::useUltimate(
        sf::Vector2f targetWorldPos)
    {
        executeAbility(
            ultimate_,
            targetWorldPos);
    }

    void AbilityComponent::executeAbility(
        const std::unique_ptr<Ability>& ability,
        sf::Vector2f targetWorldPos) const
    {
        if (!ability || !owner)
        {
            return;
        }

        ability->execute(
            owner->position,
            targetWorldPos,
            owner->velocity);
    }
}