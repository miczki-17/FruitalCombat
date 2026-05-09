#include "IntroState.h"
#include "../core/Game.h"
#include <format>

namespace game::states
{
	IntroState::IntroState(game::Game* game)
		: State(game), currentFrame(1), totalFrames(24), elapsedTime(0.f)
	{
		//std::cout << SFML_VERSION_MAJOR << "."
		//	<< SFML_VERSION_MINOR << "."
		//	<< SFML_VERSION_PATCH << std::endl;
		frameDuration = 1.0f / 30.0f;

		// intro textures pre-load
		std::cout << "[INTRO] Loading intro frames...\n";
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
				std::cerr << "[INTRO ERROR] not found: " << filename << '\n';
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
			std::cerr << "[INTRO ERROR] can not load intro music.\n";
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

		game->menuImageBuffer.clear();
		game->menuUiBuffer.clear();

		std::vector<std::string> buttonsNames = {
			"start",
			"achievements",
			"shop",
			"settings",
			"back"
		};

		// load bg
		for (int i = 1; i <= 6; ++i)
		{
			std::string filename = std::format("../../../assets/textures/menu/bg_{:01}.png", i);
			sf::Image img;
			if (img.loadFromFile(filename))
			{
				game->menuImageBuffer.push_back(std::move(img));
			}
			else
			{
				std::cerr << "[AYNC ERROR] can not find " << filename << '\n';
			}
		}
		
		// load Btns
		for (const auto& name : buttonsNames) {
			std::string filename = "../../../assets/textures/menu/" + name + ".png";
			sf::Image img;
			if (img.loadFromFile(filename))
			{
				game->menuUiBuffer[name] = std::move(img);
			}
			else
			{
				std::cerr << "[ASYNC ERROR] can not find " << filename << '\n';
			}
		}

		std::cout << "[ASYNC] menu loaded\n";
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
					std::cout << "[ASYNC] menu still loading...\n";
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