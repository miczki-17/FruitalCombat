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
        if (auto* stats = owner->getComponent<game::components::StatsComponent>())
        {
            auto& profile = game_->profile;


            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1) && profile.regularFertilizerCount > 0)
            {
                stats->addEffect(game::components::StatusType::SpeedBuff, 300.0f, 1.4f); 
                profile.regularFertilizerCount--;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2) && profile.mediumFertilizerCount > 0)
            {
                stats->addEffect(game::components::StatusType::SpeedBuff, 480.0f, 1.7f); 
                profile.mediumFertilizerCount--;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3) && profile.bestFertilizerCount > 0)
            {
                stats->addEffect(game::components::StatusType::SpeedBuff, 720.0f, 2.2f); 
                profile.bestFertilizerCount--;
            }
        }
    }
}