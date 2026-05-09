#pragma once

#include <iostream>
#include <optional>
#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace game
{
	class Game;
}


namespace game::states
{
	enum class StateType
	{
		Intro,
		Menu,
		Playing,
		Settings
	};




	class State
	{
	protected:
		game::Game* game;

	public:
		State(game::Game* game)
			: game(game) {}

		virtual ~State() = default;

		virtual void handleEvent(const sf::Event& event) = 0;
		virtual void update(float dt) = 0;
		virtual void render(sf::RenderWindow& window) = 0;

		virtual StateType getType() const = 0;
	};
}