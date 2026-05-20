#include "AcidSquirtAbility.h"
#include "../entities/Entity.h"
#include "../components/StatsComponent.h"
#include <cmath>
#include <iostream>

namespace game::components
{
    AcidSquirtAbility::AcidSquirtAbility(std::vector<game::components::Bullet>& bulletsRef, game::entities::Entity* e, std::string texturePath)
        : bullets(&bulletsRef), entity(e)
    {
        projTexture = std::make_shared<sf::Texture>();
        if (!projTexture->loadFromFile(texturePath)) {
            std::cerr << "[ERROR] Could not load texture from: " << texturePath << "\n";
        }
    }

    void AcidSquirtAbility::update(float dt)
    {
        if (currentTimer > 0.0f) currentTimer -= dt;
    }

    void AcidSquirtAbility::execute(sf::Vector2f startPos, sf::Vector2f targetWorldPos, sf::Vector2f shooterVelocity)
    {
        if (currentTimer > 0.0f || bullets == nullptr) return;

        bullets->emplace_back(startPos, sf::Vector2f(0.f, 0.f));
        auto& b = bullets->back();

        b.setAppearance(14.0f, sf::Color(255, 120, 0, 240));
        b.setStatusEffect(StatusEffect::Poison);
        b.setWobble(false, false);
        b.setupParabolic(startPos, targetWorldPos, 380.0f);

        if (projTexture->getSize().x > 0) {
            b.setAnimation(projTexture, 4, 0.1f, { 32, 32 });
        }

        // --- Czysty odczyt statystyki poprzez ECS ---
        float speedMod = 1.0f;
        if (entity != nullptr) {
            auto* stats = entity->getComponent<StatsComponent>();
            if (stats) speedMod = stats->attackSpeed;
        }

        currentTimer = baseCooldown / speedMod;
    }
}