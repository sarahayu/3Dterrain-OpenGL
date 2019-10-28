#include "Chunk.h"
#include "../Application.h"

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>

const float Chunk::SIZE = 600.f;

Chunk::Chunk(const sf::Vector2i &position, const int &lodIndex, const StitchEdge &edges, ChunkManager &app)
	: m_app(&app)
{
	m_terrainConfig.position = position;
	m_terrainConfig.size = SIZE;
	m_terrainConfig.edges = edges;
	m_terrainConfig.setLOD(lodIndex);

	m_lowLOD = m_terrainConfig.superLowLod;

	m_app->requestMapData({ [this](const std::vector<float>& heightMap) {
		m_heightMap = heightMap;
		//m_terrain.buildMesh(m_heightMap, m_terrainConfig);

		reload();

	}, m_terrainConfig });
/*
	m_waterConfig.position = position;
	m_waterConfig.size = SIZE;
	m_waterConfig.resolve();

	m_water.buildMesh(m_waterConfig);*/
}

Chunk::Chunk(const Chunk & other)
{
	std::cout << "\nCopy called";
	*this = other;
}

Chunk & Chunk::operator=(const Chunk & other)
{
	std::cout << "\nAssignment copy called";
	*this = other;
	return *this;
}

void Chunk::updateLOD(const int &lodIndex, const StitchEdge &edges)
{
	//std::cout << "\nShould be building..." << a;
	if (!m_terrain.isMeshBuilt()) return;
	if (m_terrainConfig.setLOD(lodIndex) || m_terrainConfig.edges != edges)
	{
		m_terrainConfig.edges = edges;

		if (m_lowLOD && !m_terrainConfig.superLowLod)
		{
			std::cout << "\nLOD shift! Requesting new map data...";
			m_app->requestMapData({ [this](const std::vector<float>& heightMap) {
				m_heightMap = heightMap;
				m_lowLOD = m_terrainConfig.superLowLod;
				//m_terrain.buildMesh(m_heightMap, m_terrainConfig);

				reload();

			}, m_terrainConfig });
		}
		else
			reload();

		//m_terrain.buildMesh(m_heightMap, m_terrainConfig);

	}
}

void Chunk::updateWater(const float & deltaTime)
{
	m_water.updateWater(deltaTime);
}

void Chunk::reload()
{
	m_app->requestMeshData({ [this](const TerrainMesh& mesh) {
		m_terrain = mesh;
	}, m_terrainConfig, m_heightMap, m_terrain });
}

void Chunk::renderTerrain()
{
	//std::cout << "\nShould be rendering..." << a;
	if (!m_terrain.isMeshBuilt()) return;
	if (!m_terrain.isBuffered())
		m_terrainRender = m_terrain.bufferMesh();

	glBindVertexArray(m_terrainRender.vao);
	glDrawElements(GL_TRIANGLES, m_terrainRender.indiceCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Chunk::renderWater()
{
	if (!m_water.isBuffered())
		m_waterRender = m_water.bufferMesh();

	glBindVertexArray(m_waterRender.vao);
	glDrawElements(GL_TRIANGLES, m_waterRender.indiceCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

const sf::Vector2i Chunk::getPosition() const
{
	return m_terrainConfig.position;
}

const float Chunk::getMaxHeight(const sf::Vector2i & position) const
{
	if (!m_terrain.isMeshBuilt()) return 0.f;

	sf::Vector2i absPos = position - m_terrainConfig.position;
	assert(absPos.x >= 0 && absPos.y >= 0);

	int x = glm::mix(0, TerrainConfig::HEIGHT_MAP_DIMENSIONS, (float)absPos.x / Chunk::SIZE);
	int y = glm::mix(0, TerrainConfig::HEIGHT_MAP_DIMENSIONS, (float)absPos.y / Chunk::SIZE);

	return m_heightMap[(y + TerrainConfig::TOTAL_STEPS)*(TerrainConfig::HEIGHT_MAP_DIMENSIONS_FULL + 1) + x + TerrainConfig::TOTAL_STEPS];
}

const int Chunk::getLOD() const
{
	return m_terrainConfig.lodStep;
}
//
//const Mesh Chunk::getMesh()
//{
//	return m_terrain.getMesh();
//}

void Chunk::clearData()
{
	m_terrain.deleteData();
	m_water.deleteData();
}

const glm::vec3 Chunk::getVP(const glm::vec3 & normal) const
{
	glm::vec3 result(m_terrainConfig.position.y, -5000, m_terrainConfig.position.x);

	if (normal.x > 0) result.x += SIZE;
	if (normal.y > 0) result.y += 5000 * 2;
	if (normal.z > 0) result.z += SIZE;

	return result;
}