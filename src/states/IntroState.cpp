#include "IntroState.h"
#include "../core/Game.h"
#include <format>

namespace game::states
{
	IntroState::IntroState(game::Game* game)
		: State(game), currentFrame(1), totalFrames(193), elapsedTime(0.f)
	{
		frameDuration = 1.0f / 30.0f;

		// intro textures pre-load
		std::cout << "Loading intro frames...\n";
		for (int i = 1; i <= totalFrames; ++i)
		{
			std::string filename = std::format("../../../assets/textures/intro/ezgif-frame-{:03}.jpg", i);

			sf::Texture tex;
			if (tex.loadFromFile(filename))
			{
				introTextures.push_back(std::move(tex));
			}
			else
			{
				std::cerr << "not found: " << filename << '\n';
			}
		}

		// strech + scale
		if (!introTextures.empty())
		{
			frameSprite = sf::Sprite(introTextures[0]);

			sf::Vector2f viewSize = game->getWindow().getView().getSize();
			sf::Vector2f introSize(introTextures[0].getSize());

			float scaleX = static_cast<float>(viewSize.x) / introSize.x;
			float scaleY = static_cast<float>(viewSize.y) / introSize.y;

			(*frameSprite).setScale({ scaleX, scaleY });

			(*frameSprite).setPosition({ 0.f, 0.f });
			(*frameSprite).setOrigin({ 0.f, 0.f });
		}

		// music start
		if (introMusic.openFromFile("../../../assets/audio/intro/intro.mp3"))
		{
			introMusic.play();
		}
		else
		{
			std::cerr << "can not load intro music.\n";
		}

		//async downloading
		workerThread = std::make_unique<std::thread>(&IntroState::loadMenuAssetsInBg, this);
	}

	IntroState::~IntroState()
	{
		if (workerThread && workerThread->joinable())
		{
			workerThread->join();
		}
	}

	void IntroState::loadMenuAssetsInBg()
	{
		std::cout << "[ASYNC] menu loading...\n";

		for (int i = 1; i <= 6; ++i)
		{
			std::string filename = std::format("../../../assets/textures/menu/menu_bg_{:01}.png", i);
			sf::Image img;
			if (img.loadFromFile(filename))
			{
				game->menuImageBuffer.push_back(std::move(img));
			}
		}

		std::cout << "[ASYNC] mnu loaded\n";
		isMenuLoaded = true;
	}


	StateType IntroState::getType() const
	{
		return StateType::Intro;
	}

	void IntroState::handleEvent(const sf::Event& event)
	{
		if (event.is<sf::Event::KeyPressed>())
		{
			auto keyEvent = event.getIf<sf::Event::KeyPressed>();
			if (keyEvent->code == sf::Keyboard::Key::Space || keyEvent->code == sf::Keyboard::Key::Enter)
			{
				if (isMenuLoaded)
				{
					introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
					return;
				}
				else
				{
					std::cout << "menu still loading...\n";
					// graphics ev
				}
			}
		}
	}

	void IntroState::update(float dt)
	{
		elapsedTime += dt;
		
		// get window size
		sf::Vector2f viewSize = game->getWindow().getView().getSize();

		while (elapsedTime >= frameDuration)
		{
			elapsedTime -= frameDuration;

			// Zwiêkszamy klatkê TYLKO jeœli nie dotarliœmy jeszcze do koñca
			if (currentFrame < totalFrames)
			{
				currentFrame++;
				const sf::Texture& nextTex = introTextures[currentFrame - 1];
				(*frameSprite).setTexture(nextTex, true);

				sf::Vector2f introSize(nextTex.getSize());
				float scaleX = static_cast<float>(viewSize.x) / introSize.x;
				float scaleY = static_cast<float>(viewSize.y) / introSize.y;
				(*frameSprite).setScale({ scaleX, scaleY });
			}
			else
			{
				if (isMenuLoaded)
				{
					introMusic.stop();
					game->getStateMachine().changeState(StateType::Menu);
					return;
				}
			}
		}
	}

	void IntroState::render(sf::RenderWindow& window)
	{
		if (frameSprite.has_value())
		{
			window.draw(*frameSprite);
		}
	}
}