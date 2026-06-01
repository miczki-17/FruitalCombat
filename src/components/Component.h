// --- Component.h ---

#pragma once

#include <SFML/Graphics.hpp>
#include <cstddef> // dla std::size_t

// Forward declaration
namespace game::entities { class Entity; }

namespace game::components
{
    // --- B?YSKAWICZNY SYSTEM IDENTYFIKACJI TYPÓW (Compile-time Type ID) ---
    using ComponentTypeID = std::size_t;

    // Funkcja generuj?ca kolejne unikalne ID (wywo?ywana raz dla ka?dego typu)
    inline ComponentTypeID getUniqueComponentID() noexcept
    {
        static ComponentTypeID lastID = 0;
        return lastID++;
    }

    // Szablon, który dla ka?dego typu przypisze to samo sta?e ID na zawsze
    template <typename T>
    inline ComponentTypeID getComponentTypeID() noexcept
    {
        static const ComponentTypeID typeID = getUniqueComponentID();
        return typeID;
    }

    class Component
    {
        friend class game::entities::Entity;

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