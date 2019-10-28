#include "TerrainMesh.h"
#include <algorithm>
#include <iostream>

#include "TerrainBuilder.h"
#include "NormalCalculate.h"
#include "TerrainColorer.h"

const int TerrainConfig::LOD_STEP[TOTAL_STEPS] = { 1, 2, 4, 8, 16 };
//const int LOD_STEP[TOTAL_STEPS] = { 1, 4, 8, 16, 64, 64 };

const int TerrainConfig::MAX_HEIGHT_INCR = LOD_STEP[TOTAL_STEPS - 1];
const int TerrainConfig::HEIGHT_MAP_DIMENSIONS = 128;
const int TerrainConfig::HEIGHT_MAP_DIMENSIONS_FULL = HEIGHT_MAP_DIMENSIONS + TOTAL_STEPS * 2;

//
//const Mesh TerrainMesh::getMesh()
//{
//	return{
//		&m_vertices,
//		&m_normals,
//		&m_tangents,
//		&m_texCoords,
//		&m_indices
//	};
//}

void TerrainMesh::buildMesh(const std::vector<float> &heightMap, const TerrainConfig & config)
{
	m_buffered = false;
	m_meshBuilt = true;

	m_verticesFull.clear();

	m_vertices.clear();
	m_normals.clear();
	m_texCoords.clear();

	m_indices.clear();

	TerrainBuilder::buildMesh(config, heightMap, std::ref(*this));
	NormalCalculate::calculate(config, std::ref(*this));


	typedef TerrainConfig tc;

	auto index = [&](const sf::Vector2i &pos) {
		return (pos.y * (config.dimensions + 1) + pos.x);
	};
	
	if (config.lodStep != tc::MAX_HEIGHT_INCR)
	{
		if (config.edges.south)
			for (int i = 0; i < config.dimensions / 2; i++)
			{
				const int x = i * 2 + 1;
				float newHeight = (m_vertices[index({ x - 1, 0 })].y + m_vertices[index({ x + 1, 0 })].y) / 2;
				m_vertices[index({ x, 0 })].y = newHeight;
			}
		if (config.edges.north)
			for (int i = 0; i < config.dimensions / 2; i++)
			{
				const int x = i * 2 + 1;
				float newHeight = (m_vertices[index({ x - 1, config.dimensions })].y + m_vertices[index({ x + 1, config.dimensions })].y) / 2;
				m_vertices[index({ x, config.dimensions })].y = newHeight;
			}
		if (config.edges.west)
			for (int i = 0; i < config.dimensions / 2; i++)
			{
				const int y = i * 2 + 1;
				float newHeight = (m_vertices[index({ 0,y - 1 })].y + m_vertices[index({ 0,y + 1 })].y) / 2;
				m_vertices[index({ 0,y })].y = newHeight;
			}
		if (config.edges.east)
			for (int i = 0; i < config.dimensions / 2; i++)
			{
				const int y = i * 2 + 1;
				float newHeight = (m_vertices[index({ config.dimensions,y - 1 })].y + m_vertices[index({ config.dimensions,y + 1 })].y) / 2;
				m_vertices[index({ config.dimensions,y })].y = newHeight;
			}
	}

	for (int i = 0; i < config.dimensions; i++)
		for (int j = 0; j < config.dimensions; j++)
		{
			unsigned int index = i * (config.dimensions + 1) + j;
			m_indices.insert(m_indices.end(), {
				index,
				index + config.dimensions + 1 + 1,
				index + config.dimensions + 1,

				index,
				index + 1,
				index + config.dimensions + 1 + 1
			});
		}
}

const bool TerrainMesh::isMeshBuilt() const
{
	return m_meshBuilt;
}

const bool TerrainMesh::isBuffered() const
{
	return m_buffered;
}

const RenderInfo TerrainMesh::bufferMesh()
{
	if (m_vao || m_ebo) deleteData();

	m_vao = 0;
	m_ebo = 0;

	m_buffered = true;

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//std::cout << "\n(Re?)Buffering... Now VAO " << vao;

	genVBO(0, m_vertices, 3);
	genVBO(1, m_normals, 3);
	genVBO(2, m_tangents, 3);
	genVBO(3, m_texCoords, 2);

	glGenBuffers(1, &m_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

	m_verticesFull.clear();

	m_vertices.clear();
	m_normals.clear();
	m_texCoords.clear();

	return{ m_vao,m_indices.size() };
}

void TerrainMesh::deleteData()
{
	if (m_vao) glDeleteVertexArrays(1, &m_vao);
	if (m_ebo) glDeleteBuffers(1, &m_ebo);
	glDeleteBuffers(m_vbos.size(), m_vbos.data());
	//m_vbos.clear();
}

const bool TerrainConfig::setLOD(const int & newLodIndex)
{
	heightIncr = size / HEIGHT_MAP_DIMENSIONS;
	int index = newLodIndex;
	int newStep = LOD_STEP[index];
	superLowLod = index >= SUPER_LOW_LOD_STEP;

	if (newStep != lodStep)
	{
		lodIndex = index;
		lodStep = newStep;
		dimensions = HEIGHT_MAP_DIMENSIONS / lodStep;
		fullDimensions = dimensions + 2;
		cellSize = size / dimensions;
		return true;
	}
	return false;
}
