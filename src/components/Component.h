#pragma once
#include <SFML/Graphics.hpp>

// Forward declaration
namespace game::entities { class Entity; }

namespace game::components
{
    class Component
    {
    protected:
        game::entities::Entity* owner = nullptr;

    public:
        virtual ~Component() = default;

        void setOwner(game::entities::Entity* entity) { owner = entity; }
        game::entities::Entity* getOwner() const { return owner; }

        virtual void update(float dt) {}
        virtual void render(sf::RenderWindow& window) {}
    };
}