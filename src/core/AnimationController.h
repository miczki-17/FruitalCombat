// only gameplay


#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

namespace game::components
{
	class AnimationController
	{
	private:
		sf::Texture idleTexture;
		sf::Texture walkTexture; // --- NOWA TEKSTURA ---

		const sf::Vector2i frameSize = { 96, 96 };		// 96px x 96px

		int currentFrame = 0;
		float animationTimer = 0.0f;

		int totalIdleFrames = 4;
		int totalWalkFrames = 4; // --- LICZBA KLATEK BIEGU ---

		bool isMoving = false;
		bool facingRight = true;

	public:
		AnimationController() = default;

		// Pobiera teraz dwie ścieżki
		bool loadTextures(const std::string& idlePath, const int& idleFrames, const std::string& walkPath, const int& walkFrames);

		const sf::Texture& getDefaultTexture() const { return idleTexture; }

		void setMovementState(bool moving, bool right);

		void updateAndApply(sf::Sprite& sprite, float dt);
	};
}