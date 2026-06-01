// Entity.h

#pragma once
#include <vector>
#include <memory>
#include <type_traits> 
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include "../components/Component.h"
#include "../components/TransformComponent.h"
#include "../components/AbilityComponent.h"
#include "../components/StatsComponent.h"

namespace game::entities
{
    class Entity
    {
    private:
        std::vector<std::unique_ptr<game::components::Component>> components;
        std::array<game::components::Component*, 32> componentArray{};

    public:
        bool isPendingDestroy = false;

        void destroy() { isPendingDestroy = true; }

        bool isDead() const
        {
            // 1. Jesli czas zycia sie skonczyl (np. tekst, kaluza)
            if (isPendingDestroy) return true;

            // 2. Jesli to postac, sprawdzamy jej HP
            auto typeID = game::components::getComponentTypeID<game::components::StatsComponent>();
            if (typeID < componentArray.size() && componentArray[typeID] != nullptr)
            {
                auto* stats = static_cast<game::components::StatsComponent*>(componentArray[typeID]);
                return stats->getHealth() <= 0.0f;
            }

            return false;
        }

        bool isAttacking() const
        {
            auto typeID = game::components::getComponentTypeID<game::components::AbilityComponent>();
            if (typeID < componentArray.size() && componentArray[typeID] != nullptr)
            {
                auto* abilities = static_cast<game::components::AbilityComponent*>(componentArray[typeID]);
                return abilities->isAttacking();
            }
            return false;
        }

        // --- Standardowe metody ECS ---
        template <typename T>
        void addComponent(std::unique_ptr<T> component)
        {
            component->owner = this;
            auto typeID = game::components::getComponentTypeID<T>();

            if (typeID >= componentArray.size()) {
                // Obsluga bledu lub resize (w zaleznosci od Twojej implementacji)
            }

            componentArray[typeID] = component.get();
            components.push_back(std::move(component));
        }

        template <typename T>
        T* getComponent() const
        {
            auto typeID = game::components::getComponentTypeID<T>();
            if (typeID < componentArray.size()) {
                return static_cast<T*>(componentArray[typeID]);
            }
            return nullptr;
        }

        void update(float deltaTime)
        {
            for (auto& component : components) {
                component->update(deltaTime);
            }
        }

        // Konstruktor domyslny, ktory dodaje TransformComponent 
        Entity()
        {
            addComponent(std::make_unique<game::components::TransformComponent>());
        }
        ~Entity() = default;
    };
}