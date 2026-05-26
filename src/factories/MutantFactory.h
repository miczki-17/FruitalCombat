// --- MutantFactory.h --- 

#pragma once
#include <memory>
#include "../entities/Entity.h"
#include "../core/ArenaContext.h"
#include "../core/Game.h"
#include "../genetics/DNA.h"

namespace game::factories
{
    class MutantFactory
    {
    private:
        game::ArenaContext& context;
        game::Game* game;
        const sf::Image& collisionMask;
        float mapScale;

    public:
        MutantFactory(game::ArenaContext& arenaContext,
            game::Game* gameRef,
            const sf::Image& mask,
            float scale);

        std::unique_ptr<game::entities::Entity>
            createMutant(const game::genetics::DNA& dna,
                game::entities::Entity* targetPlayer);
    };
}