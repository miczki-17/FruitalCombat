// only gameplay

#include "AnimationController.h"
#include "Game.h"

namespace game::components
{
	bool AnimationController::loadTextures(const std::string& idlePath, const int& idleFrames, const std::string& walkPath, const int& walkFrames)
	{
		if (!idleTexture.loadFromFile(idlePath))
		{
			std::cerr << "[OSTRZEŻENIE] Brak tekstury idle: " << idlePath << "\n";
			return false;
		}
		if (!walkTexture.loadFromFile(walkPath))
		{
			std::cerr << "[OSTRZEŻENIE] Brak tekstury walk: " << walkPath << "\n";
			return false;
		}

		totalIdleFrames = idleFrames;
		totalWalkFrames = walkFrames;

		return true;
	}

	void AnimationController::setMovementState(bool moving, bool right)
	{
		isMoving = moving;
		facingRight = right;
	}

	void AnimationController::updateAndApply(sf::Sprite& sprite, float dt)
	{
		// 1. ZAMIANA TEKSTURY zależnie od flagi isMoving
		if (isMoving) sprite.setTexture(walkTexture);
		else          sprite.setTexture(idleTexture);

		// 2. LOGIKA KLATEK
		int maxFrames = isMoving ? totalWalkFrames : totalIdleFrames;

		// Przyspieszamy animację chodu (0.1f) względem oddychania (0.15f)
		float frameDuration = isMoving ? 0.1f : 0.15f;

		// Ochrona na wypadek, gdybyśmy zeszli ze stanu o 6 klatkach na stan o 4 klatkach
		if (currentFrame >= maxFrames) currentFrame = 0;

		animationTimer += dt;
		if (animationTimer >= frameDuration)
		{
			animationTimer -= frameDuration;
			currentFrame++;

			if (currentFrame >= maxFrames)
			{
				currentFrame = 0;
			}
		}

		// 3. WYCINANIE I OBRACANIE (obie animacje czytamy od góry Y=0, bo są w osobnych plikach)
		int rectLeftX = currentFrame * frameSize.x;

		if (facingRight)
		{
			sprite.setTextureRect(sf::IntRect({ rectLeftX, 0 }, frameSize));
		}
		else
		{
			sprite.setTextureRect(sf::IntRect({ rectLeftX + frameSize.x, 0 }, { -frameSize.x, frameSize.y }));
		}
	}
}