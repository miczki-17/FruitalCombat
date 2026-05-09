#include "MenuState.h"
#include "../core/Game.h"
#include <format>

namespace game::states
{
	MenuState::MenuState(game::Game* game)
		: State(game), currentFrame(1), totalFrames(6), elapsedTime(0.f)
	{
		frameDuration = 1.0f / 6.0f;

		for (const auto& img : game->menuImageBuffer)
		{
			sf::Texture tex;
			if (tex.loadFromImage(img))
			{
				bgTextures.push_back(std::move(tex));
			}
		}

		game->menuImageBuffer.clear();

		// strech + scale
		if (!bgTextures.empty())
		{
			frameSprite = sf::Sprite(bgTextures[0]);

			sf::Vector2f viewSize = game->getWindow().getView().getSize();
			sf::Vector2f introSize(bgTextures[0].getSize());

			float scaleX = static_cast<float>(viewSize.x) / introSize.x;
			float scaleY = static_cast<float>(viewSize.y) / introSize.y;

			(*frameSprite).setScale({ scaleX, scaleY });

			(*frameSprite).setPosition({ 0.f, 0.f });
			(*frameSprite).setOrigin({ 0.f, 0.f });
		}

		// music start
		if (bgMusic.openFromFile("../../../assets/audio/menu/Victory_at_Canopy_Peak.mp3"))
		{
			//bgMusic.setLooping(true);
			bgMusic.play();
		}
		else
		{
			std::cerr << "can not load bg music.\n";
		}
	}


	StateType MenuState::getType() const
	{
		return StateType::Menu;
	}

	// buttons events
	void MenuState::handleEvent(const sf::Event& event)
	{

	}

	void  MenuState::update(float dt)
	{
		elapsedTime += dt;

		// get window size
		sf::Vector2f viewSize = game->getWindow().getView().getSize();

		while (elapsedTime >= frameDuration)
		{
			elapsedTime -= frameDuration;
			currentFrame++;

			if (currentFrame <= bgTextures.size())
			{
				const sf::Texture& nextTex = bgTextures[currentFrame - 1];

				(*frameSprite).setTexture(nextTex, true);

				sf::Vector2f introSize(nextTex.getSize());
				float scaleX = static_cast<float>(viewSize.x) / introSize.x;
				float scaleY = static_cast<float>(viewSize.y) / introSize.y;

				(*frameSprite).setScale({ scaleX, scaleY });
			}
			else
			{
				currentFrame = 1;
			}
		}
	}

	void MenuState::render(sf::RenderWindow& window)
	{
		window.draw(*frameSprite);
	}
}
