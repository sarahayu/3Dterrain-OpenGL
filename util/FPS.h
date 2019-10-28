#pragma once
#include <SFML\Graphics.hpp>

class FPS
{
public:
	FPS();
	const bool update(const float &deltaTime);
	const float getFPS() const;
private:
	sf::Clock m_elapsed;
	float m_statTime = 0.f;
	int m_frames = 0;
	float m_lastFPS = 0.f;
};