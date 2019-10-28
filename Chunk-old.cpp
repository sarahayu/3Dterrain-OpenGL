#include "Chunk.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include "MathUtils.h"

const int Chunk::SIZE = 600;

const int CELL_DIM_MIN = 5;
const int CELL_DIM_MAX = 30;

const glm::vec4 SAND(1.0, 0.0, 0.0, 0.0);
const glm::vec4 GRASS(0.0, 1.0, 0.0, 0.0);
const glm::vec4 STONE(0.0, 0.0, 1.0, 0.0);
const glm::vec4 SNOW(0.0, 0.0, 0.0, 1.0);

const float SNOW_LEVEL = 200.f;
const float SEA_LEVEL = 5.f;

const int TERRAIN_WIDTH = 900, TERRAIN_DISTANCE = 700;

const double SNOW_MAX_ANGLE = 0.423, GRASS_MAX_ANGLE = 0.342;

float LACUNARITY = 2.3f, GAIN = 0.48f, AMP_START = 1000.f, FREQ_START = 0.2f;
const int OCTAVES = 7;

Chunk::Chunk(const sf::Vector2i & pos, const float & scale)
	: m_pos(pos),
	m_cellDimensions(glm::mix(CELL_DIM_MIN, CELL_DIM_MAX, std::min(1.f,std::max(scale,0.f)))),
	m_cellSize((float)SIZE / m_cellDimensions),
	m_cellDimensionsFull(m_cellDimensions + 2),
	m_scale(1.f)
{
	std::cout << "\n" << m_cellDimensions;
	for (int y = 0; y <= m_cellDimensionsFull - 1; y++)
		for (int x = 0; x <= m_cellDimensionsFull - 1; x++)
		{/*
			const float x0 = m_pos.x + x * m_cellSize;
			const float x1 = m_pos.x + (x + 1) * m_cellSize;
			const float y0 = m_pos.y + y * m_cellSize;
			const float y1 = m_pos.y + (y + 1) * m_cellSize;*/

			const bool isEdge = x != 0 && x != m_cellDimensionsFull - 1  && y != 0 && y != m_cellDimensionsFull - 1;

			auto calcPerlin = [&](const float &X, const float &Y, const bool &notEdge) {
				const float xCoord = (float)m_pos.x + (X-1) * m_cellSize;
				const float yCoord = (float)m_pos.y + (Y-1) * m_cellSize;

				float value = 0.0;
				float amplitude = AMP_START;
				float frequency = FREQ_START;

				sf::Vector2f st = { xCoord / SIZE * (float)SIZE / 400, (yCoord) / SIZE * (float)SIZE / 400 };
				for (int i = 0; i < OCTAVES; i++) {
					value += amplitude * mu::perlin(st * frequency);
					frequency *= LACUNARITY;
					amplitude *= GAIN;
				}

				m_verticesFull.insert(m_verticesFull.end(), { yCoord, value, xCoord });
				if (notEdge)
				{
					m_vertices.insert(m_vertices.end(), { yCoord, value, xCoord });
					m_texCoords.insert(m_texCoords.end(), { (float)(X - 1) / m_cellDimensions, (float)(Y - 1) / m_cellDimensions });
				}
			};

			calcPerlin(x, y, isEdge);
			calcPerlin(x, y + 1, isEdge);
			calcPerlin(x + 1, y + 1, isEdge);
			calcPerlin(x + 1, y, isEdge);

		}

	//m_colors.insert(m_colors.end(), m_vertices.size(), 1.f);

	for (int y0 = 0; y0 < m_cellDimensionsFull; y0++)
		for (int x0 = 0; x0 < m_cellDimensionsFull; x0++)
		{
			int c1 = (y0*(m_cellDimensionsFull) + x0) * 3 * 4,
				c2 = c1 + 3,
				c3 = c2 + 3,
				c4 = c3 + 3;

			glm::vec3 first(glm::vec3(m_verticesFull[c1], m_verticesFull[c1 + 1], m_verticesFull[c1 + 2])
				- glm::vec3(m_verticesFull[c2], m_verticesFull[c2 + 1], m_verticesFull[c2 + 2]));
			glm::vec3 second(glm::vec3(m_verticesFull[c2], m_verticesFull[c2 + 1], m_verticesFull[c2 + 2])
				- glm::vec3(m_verticesFull[c3], m_verticesFull[c3 + 1], m_verticesFull[c3 + 2]));

			glm::vec3 normal = glm::cross(first, second);
			if (normal.y < 0) normal = -normal;

			m_faceNormalsFull.push_back(normal);

			first = glm::vec3(glm::vec3(m_verticesFull[c4], m_verticesFull[c4 + 1], m_verticesFull[c4 + 2])
				- glm::vec3(m_verticesFull[c1], m_verticesFull[c1 + 1], m_verticesFull[c1 + 2]));
			second = glm::vec3(glm::vec3(m_verticesFull[c3], m_verticesFull[c3 + 1], m_verticesFull[c3 + 2])
				- glm::vec3(m_verticesFull[c4], m_verticesFull[c4 + 1], m_verticesFull[c4 + 2]));

			normal = glm::cross(first, second);
			if (normal.y < 0) normal = -normal;

			m_faceNormalsFull.push_back(normal);
		}

	for (int y0 = 1; y0 <= m_cellDimensionsFull - 2; y0++)
	{
		for (int x0 = 1; x0 <= m_cellDimensionsFull - 2; x0++)
		{
			int x1 = x0 + 1, y1 = y0 + 1;

			auto calcNormals = [&](const int &x, const int &y) {
				int xx = x * 2;
				sf::Vector2i t1(xx - 1, y), t2(xx, y), t3(xx + 1, y),
					t4(xx - 2, y - 1), t5(xx - 1, y - 1), t6(xx, y - 1);

				glm::vec3 normal = (m_faceNormalsFull[t1.y * (m_cellDimensionsFull * 2) + t1.x]
					+ m_faceNormalsFull[t2.y * (m_cellDimensionsFull * 2) + t2.x]
					+ m_faceNormalsFull[t3.y * (m_cellDimensionsFull * 2) + t3.x]
					+ m_faceNormalsFull[t4.y * (m_cellDimensionsFull * 2) + t4.x]
					+ m_faceNormalsFull[t5.y * (m_cellDimensionsFull * 2) + t5.x]
					+ m_faceNormalsFull[t6.y * (m_cellDimensionsFull * 2) + t6.x]);

				normal.x /= 6; normal.y /= 6; normal.z /= 6;

				normal = glm::normalize(normal);

				m_normalsFull.insert(m_normalsFull.end(), { normal.x,normal.y,normal.z });
				m_normals.insert(m_normals.end(), { normal.x,normal.y,normal.z });
			};

			calcNormals(x0, y0);
			calcNormals(x0, y1);
			calcNormals(x1, y1);
			calcNormals(x1, y0);
		}
	}

	for (int y0 = 0; y0 <= m_cellDimensions - 1; y0++)
		for (int x0 = 0; x0 <= m_cellDimensions - 1; x0++)
		{
			int x1 = x0 + 1, y1 = y0 + 1;
			int c1 = (y0 * (m_cellDimensions) + x0) * 3 * 4;
			int c2 = c1 + 3;
			int c3 = c2 + 3;
			int c4 = c3 + 3;

			auto calcColor = [&](const int &index) {
				glm::vec3 vertLoc(m_vertices[index],
					m_vertices[index + 1],
					m_vertices[index + 2]);
				glm::vec3 norm(m_normals[index],
					m_normals[index + 1],
					m_normals[index + 2]);

				glm::vec4 landColor = SAND;
				if (vertLoc.y >= SNOW_LEVEL)
				{
					if (mu::noise({ (int)vertLoc.z, (int)vertLoc.x }).x <= std::min(150.f, vertLoc.y - SNOW_LEVEL) / 150.f)
					{
						if (norm.y > 0.423) landColor = SNOW;
						else landColor = STONE;
					}
					else
					{
						if (norm.y > 0.342) landColor = GRASS;
						else landColor = STONE;
					}
				}
				else if (vertLoc.y > SEA_LEVEL + 2.0)
				{

					if (norm.y > 0.342) landColor = GRASS;
					else landColor = STONE;
				}

				m_colors.insert(m_colors.end(), { landColor.x, landColor.y, landColor.z, landColor.a });
			};

			calcColor(c1);
			calcColor(c2);
			calcColor(c3);
			calcColor(c4);
		}

	for (int i = 0; i < m_vertices.size() / 3 / 4; i++)
	{
		unsigned int index = i * 4;
		m_indices.insert(m_indices.end(), {
			index,
			index + 1,
			index + 2,

			index,
			index + 2,
			index + 3
		});
	}

}

Chunk::~Chunk()
{
}

void Chunk::clearData()
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(m_vbos.size(), m_vbos.data());
	glDeleteBuffers(m_ebos.size(), m_ebos.data());
}

const Mesh Chunk::getMesh() const
{
	return{ &m_vertices, &m_colors, &m_normals, &m_texCoords, &m_indices };
}

void Chunk::render()
{

	bufferMesh();

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

const sf::Vector2i Chunk::getPosition() const
{
	return m_pos;
}

const float Chunk::getScale() const
{
	return m_scale;
}

void Chunk::bufferMesh()
{
	if (m_buffered) return;
	m_buffered = true;

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	std::cout << "\n(Re?)Buffering... Now VAO " << m_vao;

	auto genVBO = [&](const unsigned int &index, const std::vector<float> &data, const unsigned int &dimensions) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		unsigned int VBO = 0U;
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(index, dimensions, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(index);

		m_vbos.push_back(VBO);
	};

	genVBO(0, m_vertices, 3);
	genVBO(1, m_colors, 4);
	genVBO(2, m_normals, 3);
	genVBO(3, m_texCoords, 2);

	unsigned int EBO = 0U;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
	m_ebos.push_back(EBO);


	m_verticesFull.clear();
	m_colorsFull.clear();
	m_normalsFull.clear();

	m_vertices.clear();
	m_colors.clear();
	m_normals.clear();
	m_texCoords.clear();

	m_faceNormalsFull.clear();
}

const unsigned int Chunk::getVAO() const
{
	return m_vao;
}

const glm::vec3 Chunk::getVP(const glm::vec3 & normal) const
{
	glm::vec3 result(m_pos.y, -AMP_START, m_pos.x);

	if (normal.x > 0) result.x += SIZE;
	if (normal.y > 0) result.y += AMP_START * 2;
	if (normal.z > 0) result.z += SIZE;

	return result;
}
