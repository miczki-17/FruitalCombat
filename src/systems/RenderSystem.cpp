// --- RenderSystem.cpp ---

#include "RenderSystem.h"
#include "../core/ArenaContext.h"
#include "../components/TextComponent.h"
#include "../components/SpriteComponent.h"
#include "../entities/Entity.h"

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
        for (const auto& p : context_.walkParticles)
        {
            sf::RectangleShape dustShape(sf::Vector2f(p.size * 2.0f, p.size * 2.0f));
            dustShape.setOrigin({ p.size, p.size });
            dustShape.setPosition(p.position);

            sf::Color c = p.color;

            c.a = static_cast<std::uint8_t>(c.a * (p.lifetime / p.maxLifetime));
            dustShape.setFillColor(c);

            window.draw(dustShape);
        }

		// Layer 4: Juice drops (if any)
        for (auto& drop : context_.juiceDrops) drop.render(window);

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
            if (auto* textComp = entity->getComponent<game::components::TextComponent>())
            {
                textComp->render(window);
            }
        }

    }
}