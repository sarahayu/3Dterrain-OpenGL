#include "GUIStats.h"

static const float TEXT_BUFFER = 3.f;
static const float GUI_BUFFER = 5.f;

GUIStats::GUIStats(const RenderSettings & renderSettings)
	: m_renderSettings(renderSettings)
{
	m_font.loadFromFile("resources/consola.ttf");
	m_text.setFont(m_font);
	m_text.setCharacterSize(20);
	m_text.setFillColor(sf::Color::White);
	m_guiBG.setFillColor(sf::Color(0, 0, 0, 100));

	m_windowSize = *m_renderSettings.windowSize;
}

void GUIStats::render(sf::RenderWindow &window)
{
	if (*m_renderSettings.render)
	{
		window.setView(window.getDefaultView());
		m_text.setString("FPS: " + s((int)*m_renderSettings.fps)
			+ " Speed: " + s(*m_renderSettings.speed)
			+ " Location: " + s((int)(m_renderSettings.trueLocation->x)) + "," + s((int)(m_renderSettings.trueLocation->y)) + "," + s((int)(m_renderSettings.trueLocation->z))
			+ " Frozen Chunks: " + (*m_renderSettings.freezeChunks ? "true" : "false"));

		auto bounds = m_text.getGlobalBounds();
		m_guiBG.setSize({ bounds.width + TEXT_BUFFER * 2, bounds.height + TEXT_BUFFER * 2 });
		m_guiBG.setOrigin(0.f, m_guiBG.getGlobalBounds().height);
		m_text.setOrigin(0.f, bounds.height);

		m_guiBG.setPosition(GUI_BUFFER, m_windowSize.y - GUI_BUFFER);
		m_text.setPosition(m_guiBG.getPosition() + sf::Vector2f(TEXT_BUFFER, -TEXT_BUFFER-3.f));

		window.draw(m_guiBG);
		window.draw(m_text);
	}
}
