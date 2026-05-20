#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "../components/Component.h"

namespace game::entities
{
    class Entity
    {
    private:
        std::vector<std::unique_ptr<game::components::Component>> components;

    public:
        // Wspóldzielony stan encji - dostepny dla kazdego komponentu
        sf::Vector2f position;
        sf::Vector2f velocity;

        bool isMoving = false;
        bool facingRight = true;

        float overrideSpeedLimit = 0.0f;
        bool isRolling = false;
        float actionTimer = 0.0f; // Np. czas trwania dasha

        bool isDead = false;

        Entity() = default;
        ~Entity() = default;

        void addComponent(std::unique_ptr<game::components::Component> component)
        {
            component->setOwner(this);
            components.push_back(std::move(component));
        }

        template <typename T>
        T* getComponent()
        {
            for (auto& comp : components) {
                if (T* target = dynamic_cast<T*>(comp.get())) {
                    return target;
                }
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