// --- PlayerInputComponent.cpp ---

#include "PlayerInputComponent.h"
#include "../core/Game.h"
#include "../entities/Entity.h"
#include "MovementComponent.h"
#include "../utils/MathUtils.h"

namespace game::components
{
    PlayerInputComponent::PlayerInputComponent(game::Game* game)
        : game_(game)
    {
    }

    void PlayerInputComponent::update(float dt)
    {
        if (!owner || !game_) return;

        sf::Vector2f input(0.f, 0.f);

        // Odczytujemy klawisze z rebindera w klasie Game (Zgodne z SFML 3)
        if (sf::Keyboard::isKeyPressed(game_->keyUp))    input.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(game_->keyDown))  input.y += 1.f;
        if (sf::Keyboard::isKeyPressed(game_->keyLeft))  input.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(game_->keyRight)) input.x += 1.f;

        if (auto* moveComp = owner->getComponent<MovementComponent>())
        {
            moveComp->setDesiredDirection(game::utils::math::safeNormalize(input));
        }
    }
}