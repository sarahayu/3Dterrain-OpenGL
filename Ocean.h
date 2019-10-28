#pragma once
#include "chunk\WaterMesh.h"
#include "complex.h"
#include "fft.h"

class Shader;

class Ocean
{
public:
	struct ShaderInfo
	{
		Shader *shader;
		glm::mat4 model;
	};

	Ocean(const float &size);
	~Ocean();
	void update(const float &deltaTime);
	void renderAt(const ShaderInfo &info, const sf::Vector2i &position, const bool &withOcean = true);

	void deleteData();

private:
	const RenderInfo bufferMesh();

	template <class T>
	void genVBO(const unsigned int &index, const T *data, const unsigned int &size, const unsigned int &dimensions);

	float m_size;
	float m_cellSize;

	bool m_buffered = false;
	float m_elapsedDT = 0.f;
	float m_elapsedTotal = 0.f;

	unsigned int m_vao = 0;
	unsigned int m_ebo = 0;
	std::array<unsigned int, 3> m_vbos;

	struct vertex_ocean {
		GLfloat   a, b, c; // htilde0
		GLfloat  _a, _b, _c; // htilde0mk conjugate
		GLfloat  ox, oy, oz; // original position
	};

	float g = 9.81f;				// gravity constant
	static const int N = 64, Nplus1 = N + 1;				// dimension -- N should be a power of 2
	float A = 0.0000005f;				// phillips spectrum parameter -- affects heights of waves
	glm::vec2 w = glm::vec2(12.f, 12.f);				// wind parameter
	float length = 1024;				// length parameter
	std::array<complex, N * N> h_tilde;
	std::array<complex, N * N> h_tilde_slopex;
	std::array<complex, N * N> h_tilde_slopez;
	std::array<complex, N * N> h_tilde_dx;
	std::array<complex, N * N> h_tilde_dz;
	cFFT fft;				// fast fourier transform	

	std::array<glm::vec3, Nplus1 * Nplus1> m_vertices;
	std::array<glm::vec3, Nplus1 * Nplus1> m_normals;
	std::array<glm::vec2, Nplus1 * Nplus1> m_texCoords;

	std::array<unsigned int, (N * N) * 6> m_indices;

	RenderInfo m_waterInfo;

	std::array<vertex_ocean, Nplus1 * Nplus1> m_oceanVertices;			// vertices for vertex buffer object

	float dispersion(int n_prime, int m_prime);		// deep water
	complex hTilde(float t, int n_prime, int m_prime);	
	float phillips(int n_prime, int m_prime);		// phillips spectrum
	complex hTilde_0(int n_prime, int m_prime);


};

template<class T>
inline void Ocean::genVBO(const unsigned int & index, const T *data, const unsigned int &len, const unsigned int & dimensions)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	auto &VBO = m_vbos[index];
	VBO = 0U;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, len * sizeof(T), data, GL_STATIC_DRAW);

	glVertexAttribPointer(index, dimensions, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(index);

	m_vbos[index] = VBO;
}
