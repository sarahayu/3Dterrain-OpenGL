#include "Model.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>

void Model::addMesh(const Mesh & mesh)
{
	m_vertices.insert(m_vertices.end(),
		mesh.vertices->begin(), mesh.vertices->end());
	m_normals.insert(m_normals.end(),
		mesh.normals->begin(), mesh.normals->end());
	m_tangents.insert(m_tangents.end(),
		mesh.tangents->begin(), mesh.tangents->end());
	m_texCoords.insert(m_texCoords.end(),
		mesh.texCoords->begin(), mesh.texCoords->end());

	for (const unsigned int &index : *mesh.indices)
		m_indices.push_back(index + m_indiceCount);

	m_indiceCount += mesh.vertices->size()/4;
}

void Model::render()
{

	if (!m_buffered)
	{
		m_buffered = true;
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		auto genVBO = [&](const unsigned int &index, const std::vector<float> &data, const unsigned int &dimensions) {

			auto &VBO = m_vbos[index];
			VBO = 0U;
			glGenBuffers(1, &VBO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(index, dimensions, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glEnableVertexAttribArray(index);

			m_vbos[index] = VBO;
		};

		genVBO(0, m_vertices, 4);
		genVBO(1, m_normals, 3);
		genVBO(2, m_tangents, 3);
		genVBO(3, m_texCoords, 2);

		unsigned int EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
		m_ebos.push_back(EBO);
	}

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Model::clear()
{
	m_buffered = false;
	m_indiceCount = 0;

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(m_vbos.size(), m_vbos.data());
	glDeleteBuffers(m_ebos.size(), m_ebos.data());

	m_ebos.clear();

	m_vertices.clear();
	m_normals.clear();
	m_tangents.clear();
	m_texCoords.clear();
	m_indices.clear();
}
