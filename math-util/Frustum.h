#pragma once
#include <glm\glm.hpp>
#include <array>

class Chunk;

class Frustum
{
public:
	Frustum(const glm::mat4 &projView);
	const bool within(const Chunk &c) const;
private:
	std::array<glm::vec4, 6> planes;
};