#include "SettingsState.h"
#include "../core/Game.h"
#include <format>
#include <iostream>
#include <stdexcept>
#include <algorithm> // Wymagane dla std::clamp

namespace game::states
{
	SettingsState::SettingsState(game::Game* game)
		: State(game)
	{
		sf::Vector2f viewSize = game->getWindow().getDefaultView().getSize();

		// blacked bg
		darkOverlay.setSize(viewSize);
		darkOverlay.setFillColor(sf::Color(0, 0, 0, 210));

		// Font load
		if (!font.openFromFile("../../../assets/fonts/ARIAL.TTF"))
		{
			std::cerr << "[SETTINGS ERROR] can not load font\n";
		}
		std::cout << "FONT LOADED\n";

		// --- LEFT SECTION: AUDIO ---
		float leftColX = viewSize.x * 0.25f;

		audioTitle = sf::Text(font, "Audio Settings", 32);
		(*audioTitle).setPosition({ leftColX - 100.f, 150.f });

		volumeLabel = sf::Text(font, "Master Volume", 20);
		(*volumeLabel).setPosition({ leftColX - 150.f, 250.f });

		sliderTrack.setSize({ 200.f, 10.f });
		sliderTrack.setFillColor(sf::Color(100, 100, 100));
		sliderTrack.setPosition({ leftColX + 20.f, 258.f });

		sliderHandle.setSize({ 15.f, 26.f });
		sliderHandle.setFillColor(sf::Color::White);
		sliderHandle.setOrigin({ 7.5f, 8.f });
		sliderHandle.setPosition({ leftColX + 20.f + 200.f, 258.f });

		volumeValueText = sf::Text(font, "100%", 20);
		(*volumeValueText).setPosition({ leftColX + 240.f, 250.f });

		// --- RIGHT SECTION ---
		float rightColX = viewSize.x * 0.7f;

		controlsTitle = sf::Text(font, "Movement Binds", 32);
		(*controlsTitle).setPosition({ rightColX - 100.f, 150.f });

		auto setupBindRow = [&](std::optional<sf::Text>& label, std::optional<sf::Text>& btn, const std::string& labelStr, float yPos)
		{
			label = sf::Text(font, labelStr, 20);
			(*label).setPosition({ rightColX - 150.f, yPos });

			btn = sf::Text(font, "", 20);
			(*btn).setPosition({ rightColX + 50.f, yPos });
		};

		setupBindRow(upLabel, upBtnText, "Move Up", 250.f);
		setupBindRow(leftLabel, leftBtnText, "Move Left", 310.f);
		setupBindRow(downLabel, downBtnText, "Move Down", 370.f);
		setupBindRow(rightLabel, rightBtnText, "Move Right", 430.f);

		// --- BACK BUTTON ---
		backBtnText = sf::Text(font, "[ BACK ]", 28);
		sf::FloatRect bounds = (*backBtnText).getLocalBounds();
		(*backBtnText).setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
		(*backBtnText).setPosition({ viewSize.x / 2.f, viewSize.y - 100.f });

		currentVolume = sf::Listener::getGlobalVolume();

		std::cout << "TEXT CREATED\n";
	}

	StateType SettingsState::getType() const
	{
		return StateType::Settings;
	}

	void SettingsState::handleEvent(const sf::Event& event)
	{
		// POPRAWKA: Bezwzglêdnie mapujemy kursor na sta³y widok UI kamery okna!
		sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
		sf::Vector2f mousePos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());

		// key listening
		if (currentRebind != RebindTarget::None)
		{
			if (event.is<sf::Event::KeyPressed>())
			{
				auto keyEvent = event.getIf<sf::Event::KeyPressed>();

				switch (currentRebind)
				{
				case RebindTarget::Up:    game->keyUp = keyEvent->code; break;
				case RebindTarget::Left:  game->keyLeft = keyEvent->code; break;
				case RebindTarget::Down:  game->keyDown = keyEvent->code; break;
				case RebindTarget::Right: game->keyRight = keyEvent->code; break;
				default: break;
				}

				currentRebind = RebindTarget::None;
			}
			return;
		}

		// mouse listening
		if (event.is<sf::Event::MouseButtonPressed>())
		{
			auto mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
			if (mouseEvent->button == sf::Mouse::Button::Left)
			{
				if ((*backBtnText).getGlobalBounds().contains(mousePos))
				{
					game->getStateMachine().popState();
					return;
				}

				sf::FloatRect expandedSliderArea({ sliderTrack.getPosition().x - 10.f, sliderTrack.getPosition().y - 15.f }, { 220.f, 40.f });
				if (expandedSliderArea.contains(mousePos))
				{
					isDraggingSlider = true;
				}

				if ((*upBtnText).getGlobalBounds().contains(mousePos))    currentRebind = RebindTarget::Up;
				if ((*leftBtnText).getGlobalBounds().contains(mousePos))  currentRebind = RebindTarget::Left;
				if ((*downBtnText).getGlobalBounds().contains(mousePos))  currentRebind = RebindTarget::Down;
				if ((*rightBtnText).getGlobalBounds().contains(mousePos)) currentRebind = RebindTarget::Right;
			}
		}

		if (event.is<sf::Event::MouseButtonReleased>())
		{
			auto mouseEvent = event.getIf<sf::Event::MouseButtonReleased>();
			if (mouseEvent->button == sf::Mouse::Button::Left)
			{
				isDraggingSlider = false;
			}
		}
	}

	void SettingsState::update(float dt)
	{
		sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
		sf::Vector2f uiMousePos = game->getWindow().mapPixelToCoords(pixelPos, game->getWindow().getDefaultView());

		// --- slider update ---
		if (isDraggingSlider)
		{
			float trackX = sliderTrack.getPosition().x;
			float trackWidth = sliderTrack.getSize().x;

			float newVol = ((uiMousePos.x - trackX) / trackWidth) * 100.0f;
			currentVolume = std::clamp(newVol, 0.0f, 100.0f);

			sf::Listener::setGlobalVolume(currentVolume);
		}

		float handleX = sliderTrack.getPosition().x + (sliderTrack.getSize().x * (currentVolume / 100.0f));
		sliderHandle.setPosition({ handleX, sliderHandle.getPosition().y });
		(*volumeValueText).setString(std::format("{:.0f}%", currentVolume));

		// --- binds update ---
		(*upBtnText).setString(currentRebind == RebindTarget::Up ? "[ ... ]" : "[ " + keyToString(game->keyUp) + " ]");
		(*leftBtnText).setString(currentRebind == RebindTarget::Left ? "[ ... ]" : "[ " + keyToString(game->keyLeft) + " ]");
		(*downBtnText).setString(currentRebind == RebindTarget::Down ? "[ ... ]" : "[ " + keyToString(game->keyDown) + " ]");
		(*rightBtnText).setString(currentRebind == RebindTarget::Right ? "[ ... ]" : "[ " + keyToString(game->keyRight) + " ]");

		if ((*backBtnText).getGlobalBounds().contains(uiMousePos))
			(*backBtnText).setFillColor(sf::Color::Yellow);
		else
			(*backBtnText).setFillColor(sf::Color::White);
	}

	void SettingsState::render(sf::RenderWindow& window)
	{
		sf::View uiView = window.getDefaultView();
		window.setView(uiView);

		window.draw(darkOverlay);

		if (audioTitle.has_value())       window.draw(*audioTitle);
		if (volumeLabel.has_value())      window.draw(*volumeLabel);

		window.draw(sliderTrack);
		window.draw(sliderHandle);

		if (volumeValueText.has_value())  window.draw(*volumeValueText);

		if (controlsTitle.has_value())    window.draw(*controlsTitle);

		if (upLabel.has_value())          window.draw(*upLabel);
		if (upBtnText.has_value())        window.draw(*upBtnText);

		if (leftLabel.has_value())        window.draw(*leftLabel);
		if (leftBtnText.has_value())      window.draw(*leftBtnText);

		if (downLabel.has_value())        window.draw(*downLabel);
		if (downBtnText.has_value())      window.draw(*downBtnText);

		if (rightLabel.has_value())       window.draw(*rightLabel);
		if (rightBtnText.has_value())     window.draw(*rightBtnText);

		if (backBtnText.has_value())      window.draw(*backBtnText);
	}

	std::string SettingsState::keyToString(sf::Keyboard::Key key)
	{
		switch (key)
		{
		case sf::Keyboard::Key::W: return "W";
		case sf::Keyboard::Key::A: return "A";
		case sf::Keyboard::Key::S: return "S";
		case sf::Keyboard::Key::D: return "D";
		case sf::Keyboard::Key::Up: return "Up Arrow";
		case sf::Keyboard::Key::Left: return "Left Arrow";
		case sf::Keyboard::Key::Down: return "Down Arrow";
		case sf::Keyboard::Key::Right: return "Right Arrow";
		case sf::Keyboard::Key::Space: return "Space";
		default: return "Key " + std::to_string(static_cast<int>(key));
		}
	}
}