#include "WaterMesh.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>

const float FRAME_DURATION = 1.f / 30;
const float FREQUENCY = 4.f;
const float TWO_PI = 3.141592654f * 2;

void WaterMesh::buildMesh(const WaterConfig & config)
{
	m_buffered = false;
	m_config = config;

	typedef WaterConfig wc;

	for (int y = 0; y <= wc::DIMENSIONS; y++)
		for (int x = 0; x <= wc::DIMENSIONS; x++)
		{
			const float Y = y*config.cellSize, X = x*config.cellSize;
			m_vertices.push_back({
				(float)config.position.y + Y,
				std::sin(X * TWO_PI / config.size * FREQUENCY) * 5.f,
				(float)config.position.x + X
			});

			m_normals.push_back({
				0.f,
				1.f,
				-std::cos(X * TWO_PI / config.size * FREQUENCY)
			});

			m_texCoords.push_back({
				(float)x / wc::DIMENSIONS,
				(float)y / wc::DIMENSIONS
			});
		}

	for (int i = 0; i < wc::DIMENSIONS; i++)
		for (int j = 0; j <  wc::DIMENSIONS; j++)
		{
			unsigned int index = i * (wc::DIMENSIONS + 1) + j;
			m_indices.insert(m_indices.end(), {
				index,
				index + wc::DIMENSIONS + 1 + 1,
				index + wc::DIMENSIONS + 1,

				index,
				index + 1,
				index + wc::DIMENSIONS + 1 + 1
			});
		}
}

const bool WaterMesh::isBuffered() const
{
	return m_buffered;
}

const RenderInfo WaterMesh::bufferMesh()
{
	m_buffered = true;

	if (m_vao || m_ebo) deleteData();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	m_vao = 0;
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//std::cout << "\n(Re?)Buffering... Now VAO " << vao;

	genVBO(0, m_vertices, 3);
	genVBO(1, m_normals, 3);
	genVBO(2, m_texCoords, 2);

	m_ebo = 0;
	glGenBuffers(1, &m_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
	/*
	m_vertices.clear();
	m_normals.clear();
	m_texCoords.clear();*/

	return{ m_vao,m_indices.size() };
}

void WaterMesh::updateWater(const float & deltaTime)
{
	m_elapsedDT += deltaTime;
	m_elapsedTotal += deltaTime;

	typedef WaterConfig wc;

	if (m_elapsedDT >= FRAME_DURATION)
	{
		m_elapsedDT = std::fmod(m_elapsedDT, FRAME_DURATION);

		for (int y = 0; y <= wc::DIMENSIONS; y++)
			for (int x = 0; x <= wc::DIMENSIONS; x++)
			{
				const float Y = y*m_config.cellSize, X = x*m_config.cellSize;

				m_vertices[y * (wc::DIMENSIONS + 1) + x].y = std::sin((X + m_elapsedTotal * 20) * TWO_PI / m_config.size * FREQUENCY) * 5.f;

				m_normals[y * (wc::DIMENSIONS + 1) + x].y = 1.f;
				m_normals[y * (wc::DIMENSIONS + 1) + x].z = -std::cos((X + m_elapsedTotal * 20) * TWO_PI / m_config.size * FREQUENCY);
			}
	}

	m_buffered = false;
	//bufferMesh();
}

void WaterMesh::deleteData()
{
	if (m_vao) glDeleteVertexArrays(1, &m_vao);
	if (m_ebo) glDeleteBuffers(1, &m_ebo);
	glDeleteBuffers(m_vbos.size(), m_vbos.data());
}

void WaterConfig::resolve()
{
	cellSize = size / DIMENSIONS;
}
