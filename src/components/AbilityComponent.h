#pragma once

#include "Component.h"
#include "../abilities/Ability.h"

#include <memory>

namespace game::components
{
    class AbilityComponent final : public Component
    {
    public:
        void setWeapon(std::unique_ptr<Ability> weapon);
        void setSkill(std::unique_ptr<Ability> skill);
        void setUltimate(std::unique_ptr<Ability> ultimate);

        void update(float deltaTime) override;

        void useWeapon(sf::Vector2f targetWorldPos);
        void useSkill(sf::Vector2f targetWorldPos);
        void useUltimate(sf::Vector2f targetWorldPos);

    private:
        std::unique_ptr<Ability> weapon_;
        std::unique_ptr<Ability> skill_;
        std::unique_ptr<Ability> ultimate_;

        void executeAbility(
            const std::unique_ptr<Ability>& ability,
            sf::Vector2f targetWorldPos) const;
    };
}