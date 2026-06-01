// --- RenderSystem.cpp ---

#include "RenderSystem.h"
#include "../core/ArenaContext.h"
#include "../components/TextComponent.h"
#include "../components/SpriteComponent.h"
#include "../components/ParticleComponent.h"
#include "../components/TransformComponent.h"
#include "../components/JuiceComponent.h"
#include "../components/AoEComponent.h"
#include "../entities/Entity.h"

#include <SFML/Graphics/RectangleShape.hpp>

namespace game::systems
{
    RenderSystem::RenderSystem(game::ArenaContext& context)
        : context_(context)
    {
    }

    void RenderSystem::renderWorld(sf::RenderWindow& window, std::optional<sf::Sprite>& mapSprite,
        game::entities::Entity* player, std::vector<std::unique_ptr<game::entities::Entity>>& enemies)
    {
		// Layer 1: background map (if loaded successfully)
        if (mapSprite.has_value()) window.draw(*mapSprite);

		// Layer 3: Walk dust particles (if any)
        sf::RectangleShape dustShape;

        for (auto& entity : context_.entities)
        {
            auto* particle = entity->getComponent<game::components::ParticleComponent>();
            if (!particle) continue; // Encja nie jest kurzem

            auto* transform = entity->getComponent<game::components::TransformComponent>();
            if (!transform) continue; // Kurz bez pozycji? Niemo¿liwe, ale bezpieczne.

            // Konfigurujemy kszta³t na podstawie danych z komponentu
            dustShape.setSize({ particle->size * 2.0f, particle->size * 2.0f });
            dustShape.setOrigin({ particle->size, particle->size });
            dustShape.setPosition(transform->position);

            sf::Color c = particle->baseColor;
            dustShape.setFillColor(c);

            window.draw(dustShape);
        }

		// Layer 4: Juice drops (if any)
        for (auto& entity : context_.entities)
        {
            if (auto* juice = entity->getComponent<game::components::JuiceComponent>()) {
                juice->render(window);
            }
        }

		// Layer 5: Bullets (if any)
        for (auto& entity : context_.entities)
        {
            if (auto* proj = entity->getComponent<game::components::ProjectileComponent>())
            {
                proj->render(window);
            }
        }
        // Layer 6: Sprites
        for (auto& entity : context_.entities)
        {
            if (auto* spriteComp = entity->getComponent<game::components::SpriteComponent>())
            {
                spriteComp->render(window);
            }
        }

        // Layer 7: Enemies (if any)
        for (auto& enemy : enemies) {
            if (auto* sprite = enemy->getComponent<game::components::SpriteComponent>()) {
                sprite->render(window);
            }
        }

        // Layer 8: Player (if exists)
        if (player != nullptr) {
            if (auto* sprite = player->getComponent<game::components::SpriteComponent>()) {
                sprite->render(window);
            }
        }

		// Layer 9: AoE zones (if any)
        for (auto& entity : context_.entities)
        {
            if (auto* aoeComp = entity->getComponent<game::components::AoEComponent>())
            {
                aoeComp->render(window);
            }
        }

    }
}