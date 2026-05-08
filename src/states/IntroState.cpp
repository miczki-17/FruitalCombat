#include "IntroState.h"
#include "../core/Game.h"
#include <format>
#include <iostream>
#include <algorithm> // do std::min

namespace game::states
{
	IntroState::IntroState(game::Game* game)
		: State(game), currentFrame(1), totalFrames(193), elapsedTime(0.f)
	{
		frameDuration = 1.0f / 30.0f;

		// 1. PRE-LOADING TEKSTUR - TO ROBIMY NAJPIERW!
		std::cout << "Loading intro frames (this might take a second)...\n";
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

		// 2. PRZYGOTOWANIE PIERWSZEJ KLATKI, SKALOWANIE I CENTROWANIE
		if (!introTextures.empty())
		{
			frameSprite = sf::Sprite(introTextures[0]);

			// Zak³adam, ¿e w Game.h metoda zwracaj¹ca okno to getWindow()
			auto windowSize = game->getWindow().getSize();
			auto textureSize = introTextures[0].getSize();

			// ZMIANA NA std::min: Obrazek w ca³oœci zmieœci siê w oknie, zachowuj¹c proporcje
			float scaleX = static_cast<float>(windowSize.x) / textureSize.x;
			float scaleY = static_cast<float>(windowSize.y) / textureSize.y;
			float scale = std::min(scaleX, scaleY);

			// Aplikacja skali
			(*frameSprite).setScale({ scaleX, scaleY });

			// CENTROWANIE OBRAZKA (przydatne, jeœli pojawi¹ siê czarne paski po bokach)
			// Ustawiamy punkt odniesienia na œrodek samego obrazka (po przeskalowaniu)...
			(*frameSprite).setOrigin({ textureSize.x / 2.0f, textureSize.y / 2.0f });
			// ... i umieszczamy go dok³adnie na œrodku rozdzielczoœci okna
			(*frameSprite).setPosition({ windowSize.x / 2.0f, windowSize.y / 2.0f });
		}

		// 3. £ADOWANIE I START MUZYKI - NA SAMYM KOÑCU
		// Robimy to na koñcu konstruktora, aby gra zaciê³a siê na wczytywaniu obrazków,
		// a gdy bêdzie gotowa odpaliæ pierwsz¹ klatkê, muzyka wystartuje równo z ni¹!
		if (introMusic.openFromFile("../../../assets/audio/intro.mp3"))
		{
			introMusic.play();
		}
		else
		{
			std::cerr << "can not to load intro music.\n";
		}
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
				game->getStateMachine().changeState(StateType::Menu);
				introMusic.stop();
			}
		}
	}

	void IntroState::update(float dt)
	{
		elapsedTime += dt;

		while (elapsedTime >= frameDuration)
		{
			elapsedTime -= frameDuration;
			currentFrame++;

			if (currentFrame <= introTextures.size())
			{
				// Zmiana obrazka.
				// Poniewa¿ w konstruktorze u¿y³em setOrigin() i setScale(),
				// nowa tekstura wskoczy zachowuj¹c to wyœrodkowanie i rozmiar.
				(*frameSprite).setTexture(introTextures[currentFrame - 1]);
			}
			else
			{
				game->getStateMachine().changeState(StateType::Menu);
				return;
			}
		}
	}

	void IntroState::render(sf::RenderWindow& window)
	{
		window.draw(*frameSprite);
	}
}