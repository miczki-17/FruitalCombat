#include "AcidSquirtAbility.h"
#include "../components/StatsComponent.h"
#include <cmath>

namespace game::components
{
    AcidSquirtAbility::AcidSquirtAbility(std::vector<game::components::Bullet>& bulletsRef, game::entities::Entity* e, std::string texturePath)
        : bullets(&bulletsRef), entity(e) {
        projTexture = std::make_shared<sf::Texture>();
        projTexture->loadFromFile(texturePath);
    }

    void AcidSquirtAbility::update(float dt) { if (currentTimer > 0.0f) currentTimer -= dt; }

    void AcidSquirtAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity)
    {
        if (currentTimer > 0.0f) return;

        int bonusProj = (entity) ? entity->getComponent<StatsComponent>()->bonusProjectiles : 0;
        int count = 3 + bonusProj;
        float angleStep = 0.2f;

        sf::Vector2f dir = targetWorldPos - startPos;
        float baseAngle = std::atan2(dir.y, dir.x);

        for (int i = 0; i < count; ++i) {
            float angle = baseAngle + (i - count / 2.0f) * angleStep;
            bullets->emplace_back(startPos, sf::Vector2f(std::cos(angle), std::sin(angle)) * 300.0f);
            auto& b = bullets->back();
            b.setAppearance(12.0f, sf::Color(100, 255, 0, 200));
            b.setStatusEffect(StatusEffect::Poison);
            if (projTexture) b.setAnimation(projTexture, 4, 0.1f, { 32, 32 });
        }
        currentTimer = baseCooldown;
    }
}