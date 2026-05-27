// --- GameWorld.h ---

#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Image.hpp>
#include <optional>

// Forward declarations
namespace game { class Game; struct ArenaContext; }
namespace game::entities { class Entity; }
namespace game::factories { class MutantFactory; }
namespace game::systems {
    class EvolutionManager;
    class CollisionSystem;
    class CombatSystem;
    class ParticleSystem;
    class RenderSystem;
    class MapHazardSystem;
}

namespace game::core
{
    class GameWorld final
    {
    public:
        GameWorld(game::Game* game, const sf::Image& collisionMask, float mapScale, const sf::Vector2f& startPos);
        ~GameWorld();

        void update(float dt);
        void render(sf::RenderWindow& window, std::optional<sf::Sprite>& mapSprite);

        // Dostep dla PlayingState (aby moc ruszac kamera i czytac klawiature)
        game::entities::Entity* getPlayer() const { return player.get(); }
        game::systems::EvolutionManager* getEvolutionManager() const { return evolutionManager.get(); }

        bool requiresShop() const;
        void resolveShopBreak();

    private:
        game::Game* game_;
        const sf::Image& collisionMask_;
        float mapScale_;

        sf::Vector2f lastPlayerPos_;
        float playerDustSpawnTimer_ = 0.0f;

        // Posiadacz wszystkich obiektow w grze:
        std::unique_ptr<game::entities::Entity> player;
        std::vector<std::unique_ptr<game::entities::Entity>> enemies;

        // Fabryki i Menedzer Ewolucji
        std::unique_ptr<game::factories::MutantFactory> mutantFactory;
        std::unique_ptr<game::systems::EvolutionManager> evolutionManager;

        // Zgrupowane Systemy:
        std::unique_ptr<game::systems::CollisionSystem> collisionSystem;
        std::unique_ptr<game::systems::CombatSystem> combatSystem;
        std::unique_ptr<game::systems::ParticleSystem> particleSystem;
        std::unique_ptr<game::systems::RenderSystem> renderSystem;
        std::unique_ptr<game::systems::MapHazardSystem> mapHazardSystem;
    };
}