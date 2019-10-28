#pragma once
#include "RenderInfo.h"
#include <SFML\Graphics.hpp>
#include <glm\glm.hpp>
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <array>

struct WaterConfig
{
	void resolve();

	sf::Vector2i position;
	float size;

	float cellSize;

	static const int DIMENSIONS = 64;
};

class WaterMesh
{
public:
	void buildMesh(const WaterConfig &config);
	const bool isBuffered() const;
	const RenderInfo bufferMesh();

	void updateWater(const float &deltaTime);

	void deleteData();

private:
	template <class T>
	void genVBO(const unsigned int &index, const std::vector<T> &data, const unsigned int &dimensions);

	WaterConfig m_config;		// TODO change this? waterconfig remains static for the most part

	bool m_buffered = false;
	float m_elapsedDT = 0.f;
	float m_elapsedTotal = 0.f;

	unsigned int m_vao = 0;
	unsigned int m_ebo = 0;
	std::array<unsigned int, 3> m_vbos;

	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texCoords;

	std::vector<unsigned int> m_indices;
};

template<class T>
inline void WaterMesh::genVBO(const unsigned int & index, const std::vector<T>& data, const unsigned int & dimensions)
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
