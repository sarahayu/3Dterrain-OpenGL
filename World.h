#pragma once
#include "Chunk.h"

class World
{
private:
	std::map<sf::Vector2f, Chunk> m_chunks;
};