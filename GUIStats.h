#pragma once
#include <SFML\Graphics.hpp>
#include <glm\glm.hpp>

class GUIStats
{
public:
	struct RenderSettings
	{
		bool *render;
		float *fps;
		sf::Vector2i *windowSize;
		float *speed;
		glm::vec3 *trueLocation;
		bool *freezeChunks;
	};

	GUIStats(const RenderSettings &renderSettings);

	void render(sf::RenderWindow &window);

private:

	template <typename T>
	static const std::string s(const T &t);

	RenderSettings m_renderSettings;

	sf::Vector2i m_windowSize;
	sf::RectangleShape m_guiBG;
	sf::Font m_font;
	sf::Text m_text;
};

template<typename T>
inline const std::string GUIStats::s(const T & t)
{
	return std::to_string(t);
}
