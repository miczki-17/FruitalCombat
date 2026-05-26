// Entity.h

#pragma once
#include <vector>
#include <memory>
#include <type_traits> // Dla std::is_base_of_v
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include "../components/Component.h"

namespace game::entities
{
    class Entity
    {
    private:
        // Wektor fizycznie przechowuj?cy i zarz?dzaj?cy czasem ?ycia komponentów
        std::vector<std::unique_ptr<game::components::Component>> components;

        // Tablica szybkiego dost?pu (indeksem jest ID komponentu). 
        // Zero p?tli. Zero dynamic_cast. Bezpo?rednie, natychmiastowe odwo?anie.
        std::vector<game::components::Component*> componentArray;

    public:
        // --- ZMIENNE LEGACY (B?d? przenoszone do komponentów w Etapie 2 i 3) ---
        // Zostawiamy je tutaj, by system skompilowa? si? bez naruszania starego kodu
        sf::Vector2f position;
        sf::Vector2f velocity;

        bool isMoving = false;
        bool facingRight = true;

        float overrideSpeedLimit = 0.0f;
        bool isRolling = false;
        float actionTimer = 0.0f;

        bool isDead = false;
        // ------------------------------------------------------------------------

        Entity()
        {
            // Rezerwujemy z góry miejsce na 32 ró?ne komponenty.
            // Zapobiega to realokacji pami?ci podczas dzia?ania gry.
            componentArray.resize(32, nullptr);
        }

        ~Entity() = default;

        // Inteligentny addComponent, który sam bada typ wrzucanego obiektu
        template <typename T>
        void addComponent(std::unique_ptr<T> component)
        {
            // Bezpiecznik: T musi dziedziczy? po bazowym Component
            static_assert(std::is_base_of_v<game::components::Component, T>, "T musi dziedziczyc po Component!");

            component->setOwner(this);

            auto typeID = game::components::getComponentTypeID<T>();

            // Je?li przekroczyli?my zarezerwowane 32 sloty, powi?ksz tablic?
            if (typeID >= componentArray.size())
            {
                componentArray.resize(typeID + 1, nullptr);
            }

            // Zapisz szybki wska?nik
            componentArray[typeID] = component.get();

            // Przejmij w?asno?? nad pami?ci?
            components.push_back(std::move(component));
        }

        // Ekstremalnie szybki dost?p do komponentów w czasie O(1)
        template <typename T>
        T* getComponent()
        {
            auto typeID = game::components::getComponentTypeID<T>();
            if (typeID < componentArray.size())
            {
                return static_cast<T*>(componentArray[typeID]);
            }
            return nullptr;
        }

        void update(float dt)
        {
            for (auto& comp : components) comp->update(dt);
        }

        void render(sf::RenderWindow& window)
        {
            for (auto& comp : components) comp->render(window);
        }
    };
}