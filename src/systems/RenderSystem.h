// --- RenderSystem.h ---

#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace game
{
    struct ArenaContext;
}

namespace game::entities
{
    class Entity;
}

namespace game::systems
{
    class RenderSystem final
    {
    public:
        explicit RenderSystem(game::ArenaContext& context);

		// Draw all entities and world elements in the correct order. Called once per frame from PlayingState.
        void renderWorld(sf::RenderWindow& window, std::optional<sf::Sprite>& mapSprite,
            game::entities::Entity* player, std::vector<std::unique_ptr<game::entities::Entity>>& enemies);

    private:
        game::ArenaContext& context_;
    };
}