#include "FPS.h"

FPS::FPS()
{
}

const bool FPS::update(const float & deltaTime)
{
	m_statTime += deltaTime;

	m_frames++;
	if (m_statTime >= 0.5f)
	{
		m_lastFPS = m_frames / m_elapsed.restart().asSeconds();

		m_statTime -= 1.f;
		m_frames = 0;
		return true;
	}
	return false;
}

const float FPS::getFPS() const
{
	return m_lastFPS;
}
