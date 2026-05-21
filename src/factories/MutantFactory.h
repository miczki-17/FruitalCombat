#pragma once
#include <memory>
#include "../entities/Entity.h"
#include "../core/ArenaContext.h"
#include "../core/Game.h"
#include "../genetics/DNA.h"

namespace game::factories
{
    // A specialized factory dedicated solely to generating genetically modified vegetables (enemies).
    // Separating this from FruitFactory ensures strict adherence to the Single Responsibility Principle.
    class MutantFactory
    {
    private:
        game::ArenaContext& context;
        game::Game* game;
        const sf::Image& collisionMask;
        float mapScale;

    public:
        MutantFactory(game::ArenaContext& arenaContext, game::Game* gameRef, const sf::Image& mask, float scale);

        // Generates an enemy entity based entirely on its genetic code
        std::unique_ptr<game::entities::Entity> createMutant(const game::genetics::DNA& dna, game::entities::Entity* targetPlayer);
    };
}