#include "ScreenOverlay.h"
#include <glad\glad.h>

namespace
{
	const std::array<float, 12> n_vertices = {
		-1.f, -1.f,
			1.f, -1.f,
			-1.f, 1.f,

			1.f, -1.f,
			-1.f, 1.f,
			1.f, 1.f,
	};
}

void ScreenOverlay::create()
{
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	//glGenBuffers(1, &EBO);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), n_vertices.data(), GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

}

void ScreenOverlay::render()
{
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
