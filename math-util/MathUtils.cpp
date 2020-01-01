#include "MathUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// https://en.wikipedia.org/wiki/Perlin_noise
// range [-1,1]

const sf::Vector2f mu::noise(const sf::Vector2i & coords)
{
	const sf::Vector2i c = coords + RAND_INCR;
	float m = glm::dot(glm::vec2(c.x, c.y), glm::vec2(12.9898, 78.233));
	auto a = glm::fract(glm::vec2(sin(m), cos(m))* 43758.5453f) * 2.f - 1.f;
	return sf::Vector2f(a.x, a.y);
}

const float mu::perlin(const sf::Vector2f & pos)
{
	return perlin(pos.x, pos.y);
}

const float mu::perlin(const float & x, const float & y)
{
	auto dotGridGradient = [&](const int & ix, const int & iy, const float & x, const float & y) {

		const float dx = x - (float)ix;
		const float dy = y - (float)iy;
		const auto &grad = noise({ ix,iy });

		return (dx*grad.x + dy*grad.y);
	};


	auto fade = [&](const float & t) {
		return t * t * t * (t * (t * 6 - 15) + 10);
	};

	auto lerp = [&](const float & a0, const float & a1, const float & w) {
		return (1.0 - w)*a0 + w*a1;
	};

	const int x0 = std::floor(x);
	const int x1 = x0 + 1;
	const int y0 = std::floor(y);
	const int y1 = y0 + 1;

	const float sx = fade(x - (float)x0);
	const float sy = fade(y - (float)y0);

	float n0, n1;
	n0 = dotGridGradient(x0, y0, x, y);
	n1 = dotGridGradient(x1, y0, x, y);
	const float ix0 = lerp(n0, n1, sx);
	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	const float ix1 = lerp(n0, n1, sx);
	const auto result = lerp(ix0, ix1, sy);

	return result;
}
