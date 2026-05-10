#pragma once
#include "../core/State.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include "../entities/player/Player.h"

namespace game::states
{
	class PlayingState : public State
	{
	private:
		// T≥o mapy (Døungla)
		sf::Texture mapTexture;
		std::optional<sf::Sprite> mapSprite;
		sf::Vector2f mapLimits;

		// Wskaünik na gracza
		std::unique_ptr<game::entities::Player> player;

	public:
		PlayingState(game::Game* game);

		StateType getType() const override;

		void handleEvent(const sf::Event& event) override;
		void update(float dt) override;
		void render(sf::RenderWindow& window) override;
	};
}