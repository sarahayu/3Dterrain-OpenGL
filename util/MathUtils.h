#pragma once
#include <SFML\Graphics.hpp>

namespace
{
	const sf::Vector2i initRand()
	{
		srand(time(0));
		return sf::Vector2i(rand(), rand());
	}
	const sf::Vector2i RAND_INCR = initRand();
}

namespace mu
{
	const sf::Vector2f noise(const sf::Vector2i &coords);
	const float perlin(const sf::Vector2f &pos);
	const float perlin(const float & x, const float & y);
}