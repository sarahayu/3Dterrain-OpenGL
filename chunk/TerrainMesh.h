#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML\Graphics.hpp>
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <vector>
#include <array>
#include "RenderInfo.h"

#include "TerrainBuilder.h"
#include "NormalCalculate.h"

struct StitchEdge
{
	bool north, east, south, west;

	const bool operator==(const StitchEdge &other)
	{
		return other.north == north
			&& other.east == east
			&& other.south == south
			&& other.west == west;
	}

	const bool operator!=(const StitchEdge &other)
	{
		return !(*this == other);
	}
};

static const StitchEdge TOP_EDGE{ true, false, false, false };
static const StitchEdge RIGHT_EDGE{ false, true, false, false };
static const StitchEdge BOTTOM_EDGE{ false, false, true, false };
static const StitchEdge LEFT_EDGE{ false, false, false, true };

static const StitchEdge TOP_RIGHT_CORNER{ true, true, false, false };
static const StitchEdge BOTTOM_RIGHT_CORNER{ false, true, true, false };
static const StitchEdge BOTTOM_LEFT_CORNER{ false, false, true, true };
static const StitchEdge TOP_LEFT_CORNER{ true, false, false, true };

struct TerrainConfig
{
	const bool setLOD(const int &newLodIndex);		// returns true if dimensions changed

	sf::Vector2i position;
	float size;

	float heightIncr;
	int lodIndex;
	int lodStep;
	int dimensions;
	int fullDimensions;
	float cellSize;
	bool superLowLod;
	StitchEdge edges;

	static const int TOTAL_STEPS = 5;
	static const int SUPER_LOW_LOD_STEP = TOTAL_STEPS - 2;
	static const int LOD_STEP[TOTAL_STEPS];
	static const int MAX_HEIGHT_INCR;
	static const int HEIGHT_MAP_DIMENSIONS;
	static const int HEIGHT_MAP_DIMENSIONS_FULL;
};

class TerrainMesh
{
public:
	friend struct TerrainBuilder;
	friend struct NormalCalculate;

	//const Mesh getMesh();

	void buildMesh(const std::vector<float> &heightMap, const TerrainConfig &config);
	const bool isMeshBuilt() const;
	const bool isBuffered() const;
	const RenderInfo bufferMesh();

	void deleteData();

private:
	template <class T>
	void genVBO(const unsigned int &index, const std::vector<T> &data, const unsigned int &dimensions);

	bool m_buffered = false;
	bool m_meshBuilt = false;

	unsigned int m_vao = 0;
	unsigned int m_ebo = 0;
	std::array<unsigned int, 5> m_vbos;

	std::vector<glm::vec2> m_texCoordsFull;
	std::vector<glm::vec3> m_verticesFull;

	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_tangents;
	std::vector<glm::vec2> m_texCoords;

	std::vector<unsigned int> m_indices;
};

template<class T>
inline void TerrainMesh::genVBO(const unsigned int & index, const std::vector<T>& data, const unsigned int & dimensions)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	auto &VBO = m_vbos[index];
	VBO = 0U;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(index, dimensions, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(index);

	m_vbos[index] = VBO;
}
