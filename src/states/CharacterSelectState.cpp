#include "CharacterSelectState.h"
#include "../core/Game.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>

namespace
{
	sf::ConvexShape createRoundedRect(float width, float height, float radius) {
		sf::ConvexShape shape;
		const int pointsPerCorner = 8;
		shape.setPointCount(pointsPerCorner * 4);

		radius = std::min(radius, std::min(width / 2.0f, height / 2.0f));

		sf::Vector2f tr(width - radius, radius);
		sf::Vector2f br(width - radius, height - radius);
		sf::Vector2f bl(radius, height - radius);
		sf::Vector2f tl(radius, radius);

		auto addCorner = [&](int startIdx, sf::Vector2f center, float startAngle) {
			for (int i = 0; i < pointsPerCorner; ++i) {
				float angle = startAngle + (3.141592654f / 2.0f) * i / (pointsPerCorner - 1);
				shape.setPoint(startIdx + i, center + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius));
			}
		};

		addCorner(0, tr, -3.141592654f / 2.0f);
		addCorner(pointsPerCorner, br, 0.0f);
		addCorner(pointsPerCorner * 2, bl, 3.141592654f / 2.0f);
		addCorner(pointsPerCorner * 3, tl, 3.141592654f);

		return shape;
	}
}

namespace game::states
{
	CharacterSelectState::CharacterSelectState(game::Game* game)
		: State(game), targetIndex(0), currentScroll(0.0f)
	{
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float centerX = viewSize.x / 2.0f;
		float centerY = viewSize.y / 2.0f;

		if (game->menuUiBuffer.contains("select_bg"))
		{
			if (bgTex.loadFromImage(game->menuUiBuffer["select_bg"])) {
				bgSprite = sf::Sprite(bgTex);
				sf::Vector2f bgSize(bgTex.getSize());
				bgSprite->setScale({ viewSize.x / bgSize.x, viewSize.y / bgSize.y });
			}
		}

		darkOverlay.setSize(viewSize);
		darkOverlay.setFillColor(sf::Color(0, 0, 0, 80));

		if (!font.openFromFile("assets/fonts/Minecraftia-Regular.ttf")) {
			std::cerr << "[SELECT ERROR] Cannot load font.\n";
		}

		characterNameText.emplace(font, "", 45);
		characterNameText->setFillColor(sf::Color(255, 255, 255));
		characterNameText->setOutlineColor(sf::Color::Black);
		characterNameText->setOutlineThickness(4.5f);

		characterTitleText.emplace(font, "", 20);
		characterTitleText->setFillColor(sf::Color(255, 210, 120));
		characterTitleText->setOutlineColor(sf::Color::Black);
		characterTitleText->setOutlineThickness(3.0f);

		initFireflies();
		loadRoster();

		setupButton("left_arrow", leftArrowTex, leftArrowSprite, { centerX - 450.f, centerY - 50.f }, { 80.f, 80.f });
		setupButton("right_arrow", rightArrowTex, rightArrowSprite, { centerX + 450.f, centerY - 50.f }, { 80.f, 80.f });
		setupButton("choose", selectBtnTex, selectBtnSprite, { centerX, viewSize.y - 100.f }, { 170.f, 70.f });
		setupButton("back", backBtnTex, backBtnSprite, { 80.f, 50.f }, { 120.f, 60.f });
	}

	void CharacterSelectState::initFireflies()
	{
		fireflies.resize(5);
		std::random_device rd;
		std::mt19937 generator(rd());

		for (auto& f : fireflies) {
			f.lifetime = std::uniform_real_distribution<float>(0.0f, 3.0f)(generator);
			f.maxLifetime = 0.0f;
		}
	}

	void CharacterSelectState::updateFireflies(float dt)
	{
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float animTime = animationClock.getElapsedTime().asSeconds();

		float platformY = (viewSize.y / 2.0f - 50.f) + (20.0f * 3.5f);
		float platformX = viewSize.x / 2.0f;

		std::random_device rd;
		std::mt19937 generator(rd());

		for (auto& f : fireflies) {
			f.lifetime += dt;

			if (f.lifetime >= f.maxLifetime) {
				f.lifetime = 0.f;
				f.position.x = platformX + std::uniform_real_distribution<float>(-90.f, 90.f)(generator);
				f.position.y = platformY + std::uniform_real_distribution<float>(-10.f, 30.f)(generator);

				f.speed = std::uniform_real_distribution<float>(20.f, 40.f)(generator);
				f.maxLifetime = std::uniform_real_distribution<float>(2.0f, 5.0f)(generator);
				f.size = std::uniform_real_distribution<float>(1.7f, 3.0f)(generator);
				f.swayOffset = std::uniform_real_distribution<float>(0.0f, 6.28f)(generator);
			}

			f.position.y -= f.speed * dt;
			f.position.x += std::sin(animTime * 2.0f + f.swayOffset) * 15.0f * dt;

			float lifeRatio = f.lifetime / f.maxLifetime;
			f.alpha = std::sin(lifeRatio * 3.14159f) * 200.f;
		}
	}

	void CharacterSelectState::loadRoster()
	{
		roster.clear();
		

		for (auto& el : game->fruitsConfig.items())
		{
			std::string jsonKey = el.key();
			const auto& fruitData = el.value();

			FruitOption opt;
			opt.jsonKey = jsonKey;
			opt.displayName = fruitData.value("name", "Unknown Fighter");
			opt.title = fruitData.value("title", "");
			opt.hp = fruitData.value("hp", 10);
			opt.damage = fruitData.value("damage", 10);
			opt.speed = static_cast<int>(fruitData.value("maxSpeed", 300.0));

			opt.abilitiesText = "Skills: ";
			if (fruitData.contains("abilities") && fruitData["abilities"].is_array() && !fruitData["abilities"].empty()) {
				for (auto& skill : fruitData["abilities"]) {
					opt.abilitiesText += skill.get<std::string>() + ", ";
				}
				opt.abilitiesText.pop_back();
				opt.abilitiesText.pop_back();
			}
			else {
				opt.abilitiesText = "Skills: None";
			}

			opt.type = game::entities::FruitType::Apple;
			if (jsonKey == "Banana") opt.type = game::entities::FruitType::Banana;
			else if (jsonKey == "Cherry") opt.type = game::entities::FruitType::Cherry;
			else if (jsonKey == "Strawberry") opt.type = game::entities::FruitType::Strawberry;
			else if (jsonKey == "Blackberry") opt.type = game::entities::FruitType::Blackberry;
			else if (jsonKey == "Orange") opt.type = game::entities::FruitType::Orange;

			roster.push_back(std::move(opt));
			auto& savedFruit = roster.back();

			if (game->menuUiBuffer.contains("log_platform"))
			{
				if (savedFruit.platformTexture.loadFromImage(game->menuUiBuffer["log_platform"]))
				{
					savedFruit.platformSprite.emplace(savedFruit.platformTexture);
					sf::Vector2i logSize(savedFruit.platformTexture.getSize());
					savedFruit.platformSprite->setOrigin({ logSize.x / 2.0f, logSize.y / 2.0f });
				}
			}

			if (game->characterImageBuffer.contains(jsonKey))
			{
				if (savedFruit.texture.loadFromImage(game->characterImageBuffer[jsonKey]))
				{
					savedFruit.sprite.emplace(savedFruit.texture);
					sf::Vector2i size(savedFruit.texture.getSize());

					if (size.x > size.y * 2)
					{
						savedFruit.isAnimated = true;
						int frameWidth = size.x / 4;
						int frameHeight = size.y;

						for (int i = 0; i < 4; ++i) {
							savedFruit.animationFrames.push_back(sf::IntRect({ i * frameWidth, 0 }, { frameWidth, frameHeight }));
						}
						savedFruit.sprite->setTextureRect(savedFruit.animationFrames[0]);
						savedFruit.sprite->setOrigin({ frameWidth / 2.0f, frameHeight / 2.0f });
					}
					else
					{
						savedFruit.isAnimated = false;
						savedFruit.sprite->setOrigin({ size.x / 2.0f, size.y / 2.0f });
					}
				}
			}
			else
			{
				std::cerr << "[WARNING] Missing character image in RAM buffer for: " << jsonKey << "\n";
			}
		}
	}

	void CharacterSelectState::setupButton(const std::string& key, sf::Texture& tex, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
	{
		if (game->menuUiBuffer.contains(key))
		{
			if (tex.loadFromImage(game->menuUiBuffer[key]))
			{
				spr = sf::Sprite(tex);
				sf::Vector2f originalSize(tex.getSize());
				spr->setScale({ targetSize.x / originalSize.x, targetSize.y / originalSize.y });
				spr->setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });
				spr->setPosition(pos);
			}
		}
	}

	StateType CharacterSelectState::getType() const { return StateType::CharacterSelect; }

	void CharacterSelectState::handleEvent(const sf::Event& event)
	{
		if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->code == sf::Keyboard::Key::A || keyPressed->code == sf::Keyboard::Key::Left) {
				game->playUIClick(); targetIndex--;
			}
			else if (keyPressed->code == sf::Keyboard::Key::D || keyPressed->code == sf::Keyboard::Key::Right) {
				game->playUIClick(); targetIndex++;
			}
			else if (keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Space) {
				game->playUIClick();
				int N = roster.size();
				if (N > 0) {
					game->selectedFruitType = roster[(targetIndex % N + N) % N].type;
					game->getStateMachine().changeState(StateType::MapSelect);
				}
			}
			else if (keyPressed->code == sf::Keyboard::Key::Escape) {
				game->playUIClick();
				game->getStateMachine().changeState(StateType::Menu);
			}
		}

		if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
		{
			if (mousePressed->button == sf::Mouse::Button::Left)
			{
				sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
				sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);

				if (leftArrowSprite && leftArrowSprite->getGlobalBounds().contains(worldPos)) {
					game->playUIClick(); targetIndex--;
				}
				if (rightArrowSprite && rightArrowSprite->getGlobalBounds().contains(worldPos)) {
					game->playUIClick(); targetIndex++;
				}
				if (selectBtnSprite && selectBtnSprite->getGlobalBounds().contains(worldPos)) {
					game->playUIClick();
					int N = roster.size();
					if (N > 0) {
						game->selectedFruitType = roster[(targetIndex % N + N) % N].type;
						game->getStateMachine().changeState(StateType::MapSelect);
					}
				}
				if (backBtnSprite && backBtnSprite->getGlobalBounds().contains(worldPos)) {
					game->playUIClick();
					game->getStateMachine().changeState(StateType::Menu);
				}
			}
		}
	}

	void CharacterSelectState::update(float dt)
	{
		if (roster.empty()) return;

		currentScroll += (targetIndex - currentScroll) * 12.0f * dt;
		updateFireflies(dt);

		float N = static_cast<float>(roster.size());
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float centerX = viewSize.x / 2.0f;
		float centerY = viewSize.y / 2.0f - 50.f;

		for (int i = 0; i < roster.size(); ++i)
		{
			float diff = static_cast<float>(i) - currentScroll;
			while (diff < -N / 2.0f) diff += N;
			while (diff > N / 2.0f) diff -= N;
			float dist = diff;

			float xPos = centerX + dist * 320.0f;
			float yPos = centerY;

			float characterScale = std::max(1.5f, 3.5f - std::abs(dist) * 1.0f);
			float platformScale = characterScale * 0.35f;

			float alpha = std::max(0.0f, 255.0f - std::abs(dist) * 120.0f);
			float colorTint = std::max(80.0f, 255.0f - std::abs(dist) * 120.0f);

			float platformYOffset = yPos + (20.0f * characterScale);

			if (roster[i].platformSprite)
			{
				roster[i].platformSprite->setPosition({ xPos, platformYOffset });
				roster[i].platformSprite->setScale({ platformScale, platformScale });
				roster[i].platformSprite->setColor(sf::Color(colorTint, colorTint, colorTint, static_cast<std::uint8_t>(alpha)));
			}

			if (roster[i].sprite)
			{
				if (roster[i].isAnimated && std::abs(dist) < 0.5f)
				{
					roster[i].animationTimer += dt;
					if (roster[i].animationTimer >= 0.12f) {
						roster[i].animationTimer = 0.0f;
						roster[i].currentFrameIndex = (roster[i].currentFrameIndex + 1) % roster[i].animationFrames.size();
						roster[i].sprite->setTextureRect(roster[i].animationFrames[roster[i].currentFrameIndex]);
					}
				}
				else if (roster[i].isAnimated) {
					roster[i].animationTimer = 0.0f;
					roster[i].currentFrameIndex = 0;
					roster[i].sprite->setTextureRect(roster[i].animationFrames[0]);
				}

				float characterYOffset = platformYOffset - (24.0f * characterScale);
				roster[i].sprite->setPosition({ xPos, characterYOffset });
				roster[i].sprite->setScale({ characterScale, characterScale });
				roster[i].sprite->setColor(sf::Color(colorTint, colorTint, colorTint, static_cast<std::uint8_t>(alpha)));
			}
		}

		sf::Vector2i pixelPos = sf::Mouse::getPosition(game->getWindow());
		sf::Vector2f worldPos = game->getWindow().mapPixelToCoords(pixelPos);

		auto updateHover = [&](std::optional<sf::Sprite>& btn, sf::Vector2f targetSize) {
			if (!btn) return;
			sf::Vector2f texSize(btn->getTexture().getSize());
			float baseScaleX = targetSize.x / texSize.x;
			float baseScaleY = targetSize.y / texSize.y;

			if (btn->getGlobalBounds().contains(worldPos)) {
				btn->setColor(sf::Color(255, 255, 255));
				btn->setScale({ baseScaleX * 1.1f, baseScaleY * 1.1f });
			}
			else {
				btn->setColor(sf::Color(210, 210, 210));
				btn->setScale({ baseScaleX, baseScaleY });
			}
		};

		updateHover(leftArrowSprite, { 80.f, 80.f });
		updateHover(rightArrowSprite, { 80.f, 80.f });
		updateHover(selectBtnSprite, { 170.f, 70.f });
		updateHover(backBtnSprite, { 120.f, 60.f });

		int actualIndex = (targetIndex % (int)N + (int)N) % (int)N;
		if (characterNameText) {
			characterNameText->setString(roster[actualIndex].displayName);
			sf::FloatRect bounds = characterNameText->getLocalBounds();

			// SFML 3: u¿ywamy bounds.position.x/y oraz bounds.size.x/y z zaokr¹gleniem
			characterNameText->setOrigin({
				std::round(bounds.position.x + bounds.size.x / 2.0f),
				std::round(bounds.position.y + bounds.size.y / 2.0f)
				});
			// Zaokr¹glamy pozycjê X (Y i tak jest okr¹g³e w Twoim kodzie)
			characterNameText->setPosition({ std::round(centerX), 80.f });
		}

		if (characterTitleText) {
			characterTitleText->setString(roster[actualIndex].title);
			sf::FloatRect bounds = characterTitleText->getLocalBounds();

			characterTitleText->setOrigin({
				std::round(bounds.position.x + bounds.size.x / 2.0f),
				std::round(bounds.position.y + bounds.size.y / 2.0f)
				});
			characterTitleText->setPosition({ std::round(centerX), 125.f });
		}
	}

	void CharacterSelectState::drawStatBar(sf::RenderWindow& window, const std::string& label, int value, int maxValue, sf::Vector2f pos, sf::Color color)
	{
		sf::Text labelText(font, label, 16);
		labelText.setFillColor(color);
		labelText.setOutlineColor(sf::Color::Black);
		labelText.setOutlineThickness(2.0f);
		labelText.setPosition({ pos.x - 45.f, pos.y - 4.f });
		window.draw(labelText);

		float barWidth = 140.f;
		float barHeight = 10.f;
		float radius = barHeight / 2.0f;

		sf::Color bgColor(
			std::min(255, color.r + 190),
			std::min(255, color.g + 190),
			std::min(255, color.b + 190),
			200
		);

		sf::ConvexShape bgBar = createRoundedRect(barWidth, barHeight, radius);
		bgBar.setPosition(pos);
		bgBar.setFillColor(bgColor);
		window.draw(bgBar);

		float fillPercentage = std::min(1.0f, static_cast<float>(value) / static_cast<float>(maxValue));
		float currentWidth = barWidth * fillPercentage;

		if (currentWidth > 0)
		{
			sf::ConvexShape fillBar = createRoundedRect(currentWidth, barHeight, radius);
			fillBar.setPosition(pos);
			fillBar.setFillColor(color);
			window.draw(fillBar);
		}
	}

	void CharacterSelectState::render(sf::RenderWindow& window)
	{
		if (bgSprite) window.draw(*bgSprite);
		window.draw(darkOverlay);

		std::vector<std::pair<float, int>> zOrder;
		int N = roster.size();
		for (int i = 0; i < N; ++i) {
			float dist = std::abs(std::fmod(i - currentScroll + N + N / 2.0f, N) - N / 2.0f);
			zOrder.push_back({ dist, i });
		}
		std::sort(zOrder.begin(), zOrder.end(), [](auto& a, auto& b) { return a.first > b.first; });

		for (auto& item : zOrder) {
			if (roster[item.second].platformSprite) {
				window.draw(*roster[item.second].platformSprite);
			}

			if (roster[item.second].sprite) {
				sf::CircleShape shadow(20.0f);
				shadow.setOrigin({ 20.0f, 20.0f });
				float groupScale = roster[item.second].sprite->getScale().x;
				shadow.setScale({ groupScale * 1.1f, groupScale * 0.3f });

				sf::Vector2f charPos = roster[item.second].sprite->getPosition();
				shadow.setPosition({ charPos.x, charPos.y + (23.0f * groupScale) });

				sf::Color spriteColor = roster[item.second].sprite->getColor();
				shadow.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(spriteColor.a * 0.4f)));
				window.draw(shadow);
			}

			if (roster[item.second].sprite) {
				window.draw(*roster[item.second].sprite);
			}
		}

		for (const auto& f : fireflies) {
			if (f.alpha > 0.0f) {
				sf::CircleShape particle(f.size);
				particle.setOrigin({ f.size, f.size });
				particle.setPosition(f.position);
				particle.setFillColor(sf::Color(255, 235, 140, static_cast<std::uint8_t>(f.alpha)));
				window.draw(particle);
			}
		}

		int actualIndex = (targetIndex % N + N) % N;
		sf::Vector2f viewSize = game->getWindow().getView().getSize();
		float centerX = viewSize.x / 2.0f;

		float mainPlatformY = (viewSize.y / 2.0f - 50.f) + (20.0f * 3.5f);
		float baseBarY = mainPlatformY + 62.0f;
		float barStartX = centerX - 60.f;

		drawStatBar(window, "HP", roster[actualIndex].hp, 20,
			{ barStartX, baseBarY }, sf::Color(255, 110, 110));

		drawStatBar(window, "DMG", roster[actualIndex].damage, 20,
			{ barStartX, baseBarY + 30.f }, sf::Color(255, 180, 80));

		drawStatBar(window, "SPD", roster[actualIndex].speed, 500,
			{ barStartX, baseBarY + 60.f }, sf::Color(100, 200, 255));

		if (!roster.empty()) {
			sf::Text abilitiesText(font, roster[actualIndex].abilitiesText, 18);
			abilitiesText.setFillColor(sf::Color(210, 210, 210));
			abilitiesText.setOutlineColor(sf::Color::Black);
			abilitiesText.setOutlineThickness(2.5f);
			abilitiesText.setPosition({ barStartX - 45.f, baseBarY + 100.f });
			window.draw(abilitiesText);
		}

		if (characterNameText) {
			sf::Vector2f originalPos = characterNameText->getPosition();

			characterNameText->setPosition({ originalPos.x + 4.0f, originalPos.y + 4.0f });
			characterNameText->setFillColor(sf::Color(0, 0, 0, 150));
			characterNameText->setOutlineThickness(0.0f);
			window.draw(*characterNameText);

			characterNameText->setPosition(originalPos);
			characterNameText->setFillColor(sf::Color(255, 255, 255));
			characterNameText->setOutlineColor(sf::Color::Black);
			characterNameText->setOutlineThickness(4.5f);
			window.draw(*characterNameText);
		}

		if (characterTitleText) window.draw(*characterTitleText);

		if (leftArrowSprite) window.draw(*leftArrowSprite);
		if (rightArrowSprite) window.draw(*rightArrowSprite);
		if (selectBtnSprite) window.draw(*selectBtnSprite);
		if (backBtnSprite) window.draw(*backBtnSprite);
	}
}