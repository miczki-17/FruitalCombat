// --- AcidSplash.cpp ---


#include "AcidSplash.h"

#include <cstdlib>
#include <algorithm>

namespace game::effects
{
	AcidSplash::AcidSplash(
		sf::Vector2f pos,
		std::shared_ptr<sf::Texture> tex)
	{
		texture = tex;

		// Create sprite instance
		sprite.emplace(*texture);

		auto size = texture->getSize();

		sprite->setOrigin({
			size.x / 2.0f,
			size.y / 2.0f
			});

		sprite->setPosition(pos);

		// Random rotation variation
		sprite->setRotation(
			sf::degrees(static_cast<float>(rand() % 360)));

		// Random scale variation
		float scale =
			0.55f +
			static_cast<float>(rand() % 40) / 100.0f;

		targetScale = { scale, scale };

		// Initial squash/stretch scale
		startScale =
		{
			scale * 1.35f,
			scale * 0.7f
		};

		sprite->setScale(startScale);
	}

	void AcidSplash::update(float dt)
	{
		if (!active)
			return;

		lifetime += dt;

		float progress =
			std::clamp(
				lifetime / maxLifetime,
				0.0f,
				1.0f);

		// Fade out effect
		sf::Color c = sprite->getColor();

		c.a = static_cast<uint8_t>(
			255.0f * (1.0f - progress));

		sprite->setColor(c);

		// Smooth unsquash animation
		float animSpeed =
			std::min(lifetime * 10.0f, 1.0f);

		sf::Vector2f currentScale =
		{
			startScale.x +
			(targetScale.x - startScale.x) * animSpeed,

			startScale.y +
			(targetScale.y - startScale.y) * animSpeed
		};

		sprite->setScale(currentScale);

		if (progress >= 1.0f)
		{
			active = false;
		}
	}

	void AcidSplash::render(sf::RenderWindow& window)
	{
		if (active && sprite.has_value())
		{
			window.draw(*sprite);
		}
	}

	bool AcidSplash::isActive() const
	{
		return active;
	}
}