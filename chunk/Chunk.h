#pragma once
#include <SFML\Graphics.hpp>
#include "TerrainMesh.h"
#include "WaterMesh.h"

class ChunkManager;

class Chunk
{
public:
	const static float SIZE;

	Chunk(const sf::Vector2i &position, const int &lodIndex, const StitchEdge &edges, ChunkManager &app);		// TODO change to worldGen class or something
	Chunk(const Chunk &other);
	Chunk& operator=(const Chunk &other);

	void updateLOD(const int &lodIndex, const StitchEdge &edges);
	void updateWater(const float &deltaTime);
	void reload();
	void renderTerrain();
	void renderWater();

	const sf::Vector2i getPosition() const;
	const float getMaxHeight(const sf::Vector2i &position) const;
	const int getLOD() const;
	//const Mesh getMesh();
	void clearData();

	const glm::vec3 getVP(const glm::vec3 &normal) const;

private:
	ChunkManager *m_app;

	bool m_lowLOD = false;
	std::vector<float> m_heightMap;

	RenderInfo m_terrainRender;
	RenderInfo m_waterRender;

	TerrainConfig m_terrainConfig;
	WaterConfig m_waterConfig;

	TerrainMesh m_terrain;
	WaterMesh m_water;
};

struct Adjacents
{
	Chunk *north, *east, *south, *west;
};