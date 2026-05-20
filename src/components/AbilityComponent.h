#pragma once
#include "Component.h"
#include "../abilities/Ability.h"
#include "../entities/Entity.h"
#include <memory>

namespace game::components
{
    class AbilityComponent : public Component
    {
    private:
        std::unique_ptr<Ability> primaryWeapon;
        std::unique_ptr<Ability> specialSkill;

    public:
        AbilityComponent() = default;

        void setWeapon(std::unique_ptr<Ability> weapon) { primaryWeapon = std::move(weapon); }
        void setSkill(std::unique_ptr<Ability> skill) { specialSkill = std::move(skill); }

        void useWeapon(sf::Vector2f targetWorldPos)
        {
            if (primaryWeapon) primaryWeapon->execute(owner->position, targetWorldPos, owner->velocity);
        }

        void useSkill(sf::Vector2f targetWorldPos)
        {
            if (specialSkill) specialSkill->execute(owner->position, targetWorldPos, owner->velocity);
        }

        void update(float dt) override
        {
            if (primaryWeapon) primaryWeapon->update(dt);
            if (specialSkill) specialSkill->update(dt);
        }
    };
}