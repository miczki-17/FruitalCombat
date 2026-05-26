// --- RenderSystem.cpp ---

#include "RenderSystem.h"
#include "../core/ArenaContext.h"
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

		// Layer 2: Acid splashes (if any)
        for (auto& splash : context_.acidSplashes) splash.render(window);

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
        for (auto& bullet : context_.bullets) bullet.render(window);

		// Layer 6: Enemies (if any)
        for (auto& enemy : enemies) enemy->render(window);

		// Layer 7: Player (if exists)
        if (player != nullptr) player->render(window);

		// Layer 8: AoE zones (if any)
        for (auto& text : context_.floatingTexts) text.render(window);
    }
}