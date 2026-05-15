#include "CharacterSelectState.h"
#include "../core/Game.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace game::states
{
	CharacterSelectState::CharacterSelectState(game::Game* game)
		: State(game), targetIndex(0), currentScroll(0.0f)
	{
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float centerX = viewSize.x / 2.0f;
		float centerY = viewSize.y / 2.0f;

		// 1. T³o (przyciemnione dla lepszej widocznoœci)
		if (bgTex.loadFromFile("assets/textures/menu/bg_1.png")) // Mo¿esz u¿yæ t³a z menu
		{
			bgSprite = sf::Sprite(bgTex);
			sf::Vector2f bgSize(bgTex.getSize());
			bgSprite->setScale({ viewSize.x / bgSize.x, viewSize.y / bgSize.y });
			bgSprite->setColor(sf::Color(100, 100, 100)); // Przyciemnienie
		}

		// 2. Czcionka
		if (!font.openFromFile("assets/fonts/ARIAL.TTF")) // Ustaw swoj¹ czcionkê
		{
			std::cerr << "[SELECT ERROR] Cannot load font.\n";
		}
		characterNameText.emplace(font, "", 50); // SFML 3: czcionka, tekst, rozmiar
		characterNameText->setFillColor(sf::Color::White);
		characterNameText->setOutlineColor(sf::Color::Black);
		characterNameText->setOutlineThickness(3.0f);

		// 3. Wczytanie postaci
		loadRoster();

		// 4. Przyciski UI (Strza³ki i zatwierdzenie)
		setupButton("assets/textures/ui/left_arrow.png", leftArrowTex, leftArrowSprite, { centerX - 400.f, centerY }, { 80.f, 80.f });
		setupButton("assets/textures/ui/right_arrow.png", rightArrowTex, rightArrowSprite, { centerX + 400.f, centerY }, { 80.f, 80.f });
		setupButton("assets/textures/ui/choose.png", selectBtnTex, selectBtnSprite, { centerX, viewSize.y - 120.f }, { 170.f, 70.f });
		setupButton("assets/textures/menu/back.png", backBtnTex, backBtnSprite, { 100.f, 80.f }, { 120.f, 60.f });
	}

	void CharacterSelectState::loadRoster()
	{
		// character roster selection list
		std::vector<std::tuple<game::entities::FruitType, std::string, std::string, std::string>> fruitsToLoad = {
			{ game::entities::FruitType::Apple, "apple", "Gala The Gladiator", "assets/textures/entities/characters/apple.png" },
			{ game::entities::FruitType::Banana, "banana", "Bananarang", "assets/textures/characters/banana.png" },
			{ game::entities::FruitType::Cherry, "cherry", "Pit & Spit", ""},
			{ game::entities::FruitType::Strawberry, "strawberry", "juice Lee", ""},
			{ game::entities::FruitType::Blackberry, "blackberry", "Black-Brusier", ""},
			{ game::entities::FruitType::Orange, "orange", "Citrus Maximus", ""}
		};


		// 1. KLUCZ DO NAPRAWY: Zarezerwowanie pamiêci z góry.
		// Dziêki temu wektor nie bêdzie "przerzuca³" postaci w pamiêci przy dodawaniu nowych.
		roster.reserve(fruitsToLoad.size());

		for (const auto& [type, jsonKey, name, texPath] : fruitsToLoad)
		{
			// 2. Tworzymy opcjê i ustawiamy dane tekstowe/liczbowe
			FruitOption opt;
			opt.type = type;
			opt.jsonKey = jsonKey;
			opt.displayName = name;

			// load statistics
			/*if (game->fruitsConfig.contains(jsonKey)) {
				opt.hp = game->fruitsConfig[jsonKey].value("hp", 100);
				opt.damage = game->fruitsConfig[jsonKey].value("damage", 10);
				opt.speed = game->fruitsConfig[jsonKey].value("speed", 5);
			}
			else {
				opt.hp = 100; opt.damage = 10; opt.speed = 5;
			}*/

			// 3. Wrzucamy do wektora (na razie BEZ tekstury i sprite'a)
			roster.push_back(std::move(opt));

			// 4. Pobieramy referencjê do owocu, który JU¯ ZNAJDUJE SIÊ w wektorze
			auto& savedFruit = roster.back();

			// 5. Dopiero tutaj ³adujemy grafikê. Sprite podepnie siê pod stabilny adres pamiêci wewn¹trz wektora!
			if (savedFruit.texture.loadFromFile(texPath))
			{
				savedFruit.sprite.emplace(savedFruit.texture);

				sf::Vector2f size(savedFruit.texture.getSize());
				savedFruit.sprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
			}
			else
			{
				std::cerr << "[SELECT ERROR] Failed to load character texture: " << texPath << "\n";
			}
		}
	}

	void CharacterSelectState::setupButton(const std::string& path, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
	{
		if (tex.loadFromFile(path))
		{
			spr = sf::Sprite(tex);
			sf::Vector2f originalSize(tex.getSize());
			spr->setScale({ targetSize.x / originalSize.x, targetSize.y / originalSize.y });
			spr->setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });
			spr->setPosition(pos);
		}
	}

	StateType CharacterSelectState::getType() const
	{
		return StateType::CharacterSelect;
	}

	void CharacterSelectState::handleEvent(const sf::Event& event)
	{
		if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
		{
			if (mousePressed->button == sf::Mouse::Button::Left)
			{
				sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
				sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);

				if (leftArrowSprite && leftArrowSprite->getGlobalBounds().contains(worldPos))
				{
					game->playUIClick();
					targetIndex--;
				}
				if (rightArrowSprite && rightArrowSprite->getGlobalBounds().contains(worldPos))
				{
					game->playUIClick();
					targetIndex++;
				}
				if (selectBtnSprite && selectBtnSprite->getGlobalBounds().contains(worldPos))
				{
					game->playUIClick();

					// Obliczamy prawdziwy indeks (modulo radzi sobie z ujemnymi)
					int N = roster.size();
					int actualIndex = (targetIndex % N + N) % N;

					game->selectedFruitType = roster[actualIndex].type;
					game->getStateMachine().changeState(StateType::Playing);
					game->menuImageBuffer.clear();
					game->menuUiBuffer.clear();
				}
				if (backBtnSprite && backBtnSprite->getGlobalBounds().contains(worldPos))
				{
					game->playUIClick();
					game->getStateMachine().popState();
				}
			}
		}
	}

	void CharacterSelectState::update(float dt)
	{
		if (roster.empty()) return;

		// 1. P³ynne przewijanie (Lerp)
		// Im wy¿szy mno¿nik, tym szybsza animacja doje¿d¿ania na miejsce
		currentScroll += (targetIndex - currentScroll) * 10.0f * dt;

		// 2. Aktualizacja pozycji ka¿dej postaci
		float N = static_cast<float>(roster.size());
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float centerX = viewSize.x / 2.0f;
		float centerY = viewSize.y / 2.0f - 50.f; // Lekko podniesione centrum

		for (int i = 0; i < roster.size(); ++i)
		{
			// Obliczanie cyklicznej odleg³oœci od aktualnego "œrodka kamery"
			float dist = std::fmod(i - currentScroll + N + N / 2.0f, N) - N / 2.0f;

			// dist == 0 -> postaæ na œrodku
			// dist == 1 -> postaæ po prawej
			// dist == -1 -> postaæ po lewej

			// Pozycja X
			float xPos = centerX + dist * 250.0f; // 350 to odleg³oœæ miêdzy postaciami

			// Skala (najwiêksza na œrodku, maleje po bokach)
			float scale = std::max(0.5f, 1.2f - std::abs(dist) * 0.4f);

			// Przezroczystoœæ i przyciemnienie postaci z ty³u
			float alpha = std::max(0.0f, 255.0f - std::abs(dist) * 150.0f);
			float colorTint = std::max(100.0f, 255.0f - std::abs(dist) * 100.0f);

			if (roster[i].sprite)
			{
				roster[i].sprite->setPosition({ xPos, centerY });
				roster[i].sprite->setScale({ scale, scale });
				roster[i].sprite->setColor(sf::Color(colorTint, colorTint, colorTint, alpha));
			}
		}

		// 3. Hover przycisków
		sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
		sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);

		auto updateHover = [&](std::optional<sf::Sprite>& btn) {
			if (!btn) return;
			if (btn->getGlobalBounds().contains(worldPos)) btn->setColor(sf::Color(200, 200, 200));
			else btn->setColor(sf::Color::White);
		};
		updateHover(leftArrowSprite); updateHover(rightArrowSprite); updateHover(selectBtnSprite); updateHover(backBtnSprite);

		
		int actualIndex = (targetIndex % (int)N + (int)N) % (int)N;
		if (characterNameText)
		{
			characterNameText->setString(roster[actualIndex].displayName);
			sf::FloatRect bounds = characterNameText->getLocalBounds();
			characterNameText->setOrigin({ bounds.size.x / 2.0f, bounds.size.y / 2.0f });
			characterNameText->setPosition({ centerX, 100.f });
		}
	}

	void CharacterSelectState::drawStatBar(sf::RenderWindow& window, const std::string& label, int value, int maxValue, sf::Vector2f pos, sf::Color color)
	{
		// Tekst etykiety (np. "HP")
		sf::Text labelText(font, label, 20);
		labelText.setFont(font);
		labelText.setString(label);
		labelText.setCharacterSize(20);
		labelText.setFillColor(sf::Color::White);
		labelText.setOutlineColor(sf::Color::Black);
		labelText.setOutlineThickness(2.0f);
		labelText.setPosition({ pos.x - 70.f, pos.y - 5.f });
		window.draw(labelText);

		// T³o paska (ciemne)
		sf::RectangleShape bgBar({ 200.f, 20.f });
		bgBar.setPosition(pos);
		bgBar.setFillColor(sf::Color(50, 30, 20, 200)); // Drewniany, ciemny kolor
		bgBar.setOutlineColor(sf::Color::Black);
		bgBar.setOutlineThickness(2.f);

		// Pasek wartoœci
		float fillPercentage = std::min(1.0f, static_cast<float>(value) / maxValue);
		sf::RectangleShape fillBar({ 200.f * fillPercentage, 20.f });
		fillBar.setPosition(pos);
		fillBar.setFillColor(color);

		window.draw(bgBar);
		window.draw(fillBar);
	}

	void CharacterSelectState::render(sf::RenderWindow& window)
	{
		if (bgSprite) window.draw(*bgSprite);

		// 1. Renderowanie postaci (¿eby postaæ w centrum zas³ania³a te z ty³u)
		// Najpierw rysujemy te z ty³u, a na koniec centraln¹
		std::vector<std::pair<float, int>> zOrder;
		int N = roster.size();
		for (int i = 0; i < N; ++i) {
			float dist = std::abs(std::fmod(i - currentScroll + N + N / 2.0f, N) - N / 2.0f);
			zOrder.push_back({ dist, i });
		}
		// Sortowanie malej¹co po dystansie (najdalsze najpierw)
		std::sort(zOrder.begin(), zOrder.end(), [](auto& a, auto& b) { return a.first > b.first; });

		for (auto& item : zOrder) {
			if (roster[item.second].sprite) {
				window.draw(*roster[item.second].sprite);
			}
		}


		int actualIndex = (targetIndex % N + N) % N;
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float centerX = viewSize.x / 2.0f;

		// Paski statystyk
		//drawStatBar(window, "HP", roster[actualIndex].hp, 200, { centerX - 80.f, viewSize.y - 320.f }, sf::Color(200, 50, 50));
		//drawStatBar(window, "DMG", roster[actualIndex].damage, 50, { centerX - 80.f, viewSize.y - 280.f }, sf::Color(200, 150, 50));
		//drawStatBar(window, "SPD", roster[actualIndex].speed, 20, { centerX - 80.f, viewSize.y - 240.f }, sf::Color(50, 150, 200));

		if (characterNameText) window.draw(*characterNameText);

		// 3. UI
		if (leftArrowSprite) window.draw(*leftArrowSprite);
		if (rightArrowSprite) window.draw(*rightArrowSprite);
		if (selectBtnSprite) window.draw(*selectBtnSprite);
		if (backBtnSprite) window.draw(*backBtnSprite);
	}
}