#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <SFML\Graphics.hpp>
#include "Model.h"

class Chunk
{
public:
	static const int SIZE;

	Chunk(const sf::Vector2i &pos, const float &scale);
	~Chunk();

	void clearData();
	const Mesh getMesh() const;

	void render();

	const sf::Vector2i getPosition() const;
	const float getScale() const;

	const unsigned int getVAO() const;

	const glm::vec3 getVP(const glm::vec3 &normal) const;

private:
	void bufferMesh();

	unsigned int m_stitchedSides = 0;
	sf::Vector2i m_pos;		// BOTTOM LEFT!
	int m_cellDimensions;
	int m_cellDimensionsFull;
	float m_cellSize;
	float m_scale;

	unsigned int m_vao = 0U;
	bool m_buffered = false;
	std::vector<unsigned int> m_vbos;
	std::vector<unsigned int> m_ebos;

	std::vector<float> m_verticesFull;
	std::vector<float> m_colorsFull;
	std::vector<float> m_normalsFull;

	std::vector<float> m_vertices;
	std::vector<float> m_colors;
	std::vector<float> m_normals;
	std::vector<float> m_texCoords;

	std::vector<glm::vec3> m_faceNormalsFull;
	std::vector<unsigned int> m_indices;
};