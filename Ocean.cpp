#include "Ocean.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

//static const int DIMENSIONS = 64;
static const float FRAME_DURATION = 1.f / 30;
static const float PI = 3.141592654f;
static const float TWO_PI = PI * 2;
static const int REPEAT = 2;

Ocean::Ocean(const float &size)
	: fft(N)
{
	m_size = size;
	m_cellSize = size / N;

	for (int y = 0; y <= N; y++)
		for (int x = 0; x <= N; x++)
		{
			m_texCoords[y * Nplus1 + x] = {
				(float)x / N,
				(float)y / N
			};
		}

	int offset = 0;
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			unsigned int index = i * (N + 1) + j;
			m_indices[offset++] = index;
			m_indices[offset++] = index + Nplus1 + 1;
			m_indices[offset++] = index + 1;
			m_indices[offset++] = index;
			m_indices[offset++] = index + Nplus1;
			m_indices[offset++] = index + Nplus1 + 1;
		}


	int index;
	complex htilde0, htilde0mk_conj;
	for (int m_prime = 0; m_prime < Nplus1; m_prime++) {
		for (int n_prime = 0; n_prime < Nplus1; n_prime++) {
			index = m_prime * Nplus1 + n_prime;

			htilde0 = hTilde_0(n_prime, m_prime);
			htilde0mk_conj = hTilde_0(-n_prime, -m_prime).conj();

			m_oceanVertices[index].a = htilde0.a;
			m_oceanVertices[index].b = htilde0.b;
			m_oceanVertices[index]._a = htilde0mk_conj.a;
			m_oceanVertices[index]._b = htilde0mk_conj.b;

			m_oceanVertices[index].ox = m_vertices[index].x = (float)n_prime / N * size / REPEAT;
			m_oceanVertices[index].oy = m_vertices[index].y = 0.0f;
			m_oceanVertices[index].oz = m_vertices[index].z = (float)m_prime / N * size / REPEAT;

			m_normals[index].x = 0.0f;
			m_normals[index].y = 1.0f;
			m_normals[index].z = 0.0f;
		}
	}


}

Ocean::~Ocean()
{
}

void Ocean::update(const float & deltaTime)
{
	m_elapsedDT += deltaTime;
	m_elapsedTotal += deltaTime;

	if (m_elapsedDT >= FRAME_DURATION)
	{
		m_elapsedDT = std::fmod(m_elapsedDT, FRAME_DURATION);

		float kx, kz, len, lambda = -1.0f;
		int index, index1;

		for (int m_prime = 0; m_prime < N; m_prime++) {
			kz = PI * (2.0f * m_prime - N) / length;
			for (int n_prime = 0; n_prime < N; n_prime++) {
				kx = PI*(2 * n_prime - N) / length;
				len = sqrt(kx * kx + kz * kz);
				index = m_prime * N + n_prime;

				h_tilde[index] = hTilde(m_elapsedTotal, n_prime, m_prime);
				h_tilde_slopex[index] = h_tilde[index] * complex(0, kx);
				h_tilde_slopez[index] = h_tilde[index] * complex(0, kz);
				if (len < 0.000001f) {
					h_tilde_dx[index] = complex(0.0f, 0.0f);
					h_tilde_dz[index] = complex(0.0f, 0.0f);
				}
				else {
					h_tilde_dx[index] = h_tilde[index] * complex(0, -kx / len);
					h_tilde_dz[index] = h_tilde[index] * complex(0, -kz / len);
				}
			}
		}

		for (int m_prime = 0; m_prime < N; m_prime++) {
			fft.fft(h_tilde.data(), h_tilde.data(), 1, m_prime * N);
			fft.fft(h_tilde_slopex.data(), h_tilde_slopex.data(), 1, m_prime * N);
			fft.fft(h_tilde_slopez.data(), h_tilde_slopez.data(), 1, m_prime * N);
			fft.fft(h_tilde_dx.data(), h_tilde_dx.data(), 1, m_prime * N);
			fft.fft(h_tilde_dz.data(), h_tilde_dz.data(), 1, m_prime * N);
		}
		for (int n_prime = 0; n_prime < N; n_prime++) {
			fft.fft(h_tilde.data(), h_tilde.data(), N, n_prime);
			fft.fft(h_tilde_slopex.data(), h_tilde_slopex.data(), N, n_prime);
			fft.fft(h_tilde_slopez.data(), h_tilde_slopez.data(), N, n_prime);
			fft.fft(h_tilde_dx.data(), h_tilde_dx.data(), N, n_prime);
			fft.fft(h_tilde_dz.data(), h_tilde_dz.data(), N, n_prime);
		}

		int sign;
		float signs[] = { 1.0f, -1.0f };
		glm::vec3 n;
		for (int m_prime = 0; m_prime < N; m_prime++) {
			for (int n_prime = 0; n_prime < N; n_prime++) {
				index = m_prime * N + n_prime;		// index into h_tilde..
				index1 = m_prime * Nplus1 + n_prime;	// index into vertices

				sign = signs[(n_prime + m_prime) & 1];

				h_tilde[index] = h_tilde[index] * sign;

				// height
				m_vertices[index1].y = h_tilde[index].a;

				// displacement
				h_tilde_dx[index] = h_tilde_dx[index] * sign;
				h_tilde_dz[index] = h_tilde_dz[index] * sign;
				m_vertices[index1].x = m_oceanVertices[index1].ox + h_tilde_dx[index].a * lambda;
				m_vertices[index1].z = m_oceanVertices[index1].oz + h_tilde_dz[index].a * lambda;

				// normal
				h_tilde_slopex[index] = h_tilde_slopex[index] * sign;
				h_tilde_slopez[index] = h_tilde_slopez[index] * sign;
				n = glm::normalize(glm::vec3(0.0f - h_tilde_slopex[index].a, 1.0f, 0.0f - h_tilde_slopez[index].a));
				m_normals[index1].x = n.x;
				m_normals[index1].y = n.y;
				m_normals[index1].z = n.z;

				// for tiling
				if (n_prime == 0 && m_prime == 0) {
					m_vertices[index1 + N + Nplus1 * N].y = h_tilde[index].a;
					
					m_vertices[index1 + N + Nplus1 * N].x = m_oceanVertices[index1 + N + Nplus1 * N].ox + h_tilde_dx[index].a * lambda;
					m_vertices[index1 + N + Nplus1 * N].z = m_oceanVertices[index1 + N + Nplus1 * N].oz + h_tilde_dz[index].a * lambda;

					m_normals[index1 + N + Nplus1 * N].x = n.x;
					m_normals[index1 + N + Nplus1 * N].y = n.y;
					m_normals[index1 + N + Nplus1 * N].z = n.z;
				}
				if (n_prime == 0) {
					m_vertices[index1 + N].y = h_tilde[index].a;

					m_vertices[index1 + N].x = m_oceanVertices[index1 + N].ox + h_tilde_dx[index].a * lambda;
					m_vertices[index1 + N].z = m_oceanVertices[index1 + N].oz + h_tilde_dz[index].a * lambda;

					m_normals[index1 + N].x = n.x;
					m_normals[index1 + N].y = n.y;
					m_normals[index1 + N].z = n.z;
				}
				if (m_prime == 0) {
					m_vertices[index1 + Nplus1 * N].y = h_tilde[index].a;

					m_vertices[index1 + Nplus1 * N].x = m_oceanVertices[index1 + Nplus1 * N].ox + h_tilde_dx[index].a * lambda;
					m_vertices[index1 + Nplus1 * N].z = m_oceanVertices[index1 + Nplus1 * N].oz + h_tilde_dz[index].a * lambda;

					m_normals[index1 + Nplus1 * N].x = n.x;
					m_normals[index1 + Nplus1 * N].y = n.y;
					m_normals[index1 + Nplus1 * N].z = n.z;
				}
			}
		}



		m_buffered = false;
	}

}

void Ocean::renderAt(const ShaderInfo &info, const sf::Vector2i & position, const bool &withOcean)
{
	glm::mat4 model = info.model;

	if (!m_buffered)
		m_waterInfo = bufferMesh();

	glBindVertexArray(m_waterInfo.vao);

	glm::mat4 model1;

	if (withOcean)
	{
		for (int i = 0; i < REPEAT; i++)
			for (int j = 0; j < REPEAT; j++)
			{
				model1 = glm::translate(model, { position.y + m_size / REPEAT * j, 0.f, position.x + m_size / REPEAT * i });

				info.shader->setMat4("model", model1);
				glDrawElements(GL_TRIANGLES, m_waterInfo.indiceCount, GL_UNSIGNED_INT, 0);
			}
	}
	else
	{
		model1 = glm::scale(model, glm::vec3(REPEAT, 0.f, REPEAT));
		model1 = glm::translate(model1, glm::vec3( position.y, 0.f, position.x ) / (float)REPEAT);
		info.shader->setMat4("model", model1);
		glDrawElements(GL_TRIANGLES, m_waterInfo.indiceCount, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
}

void Ocean::deleteData()
{
	if (m_vao) glDeleteVertexArrays(1, &m_vao);
	if (m_ebo) glDeleteBuffers(1, &m_ebo);
	glDeleteBuffers(m_vbos.size(), m_vbos.data());
}

const RenderInfo Ocean::bufferMesh()
{
	m_buffered = true;

	if (m_vao || m_ebo) deleteData();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	m_vao = 0;
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	genVBO(0, m_vertices.data(), m_vertices.size(), 3);
	genVBO(1, m_normals.data(), m_normals.size(), 3);
	genVBO(2, m_texCoords.data(), m_texCoords.size(), 2);

	m_ebo = 0;
	glGenBuffers(1, &m_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

	return{ m_vao,m_indices.size() };
}

float Ocean::dispersion(int n_prime, int m_prime)
{
	float w_0 = 2.0f * PI / 200.0f;
	float kx = PI * (2 * n_prime - N) / length;
	float kz = PI * (2 * m_prime - N) / length;
	return floor(sqrt(g * sqrt(kx * kx + kz * kz)) / w_0) * w_0;

}

complex Ocean::hTilde(float t, int n_prime, int m_prime)
{
	int index = m_prime * Nplus1 + n_prime;

	complex htilde0(m_oceanVertices[index].a, m_oceanVertices[index].b);
	complex htilde0mkconj(m_oceanVertices[index]._a, m_oceanVertices[index]._b);

	float omegat = dispersion(n_prime, m_prime) * t;

	float cos_ = cos(omegat);
	float sin_ = sin(omegat);

	complex c0(cos_, sin_);
	complex c1(cos_, -sin_);

	complex res = htilde0 * c0 + htilde0mkconj * c1;

	return htilde0 * c0 + htilde0mkconj*c1;

}

float Ocean::phillips(int n_prime, int m_prime)
{
	glm::vec2 k(PI * (2 * n_prime - N) / length,
		PI * (2 * m_prime - N) / length);
	float k_length = glm::length(k);
	if (k_length < 0.000001) return 0.0;

	float k_length2 = k_length  * k_length;
	float k_length4 = k_length2 * k_length2;

	auto nk = glm::normalize(k), nw = glm::normalize(w);
	float k_dot_w = nk.x * nw.x + nk.y * nw.y;
	float k_dot_w2 = k_dot_w * k_dot_w * k_dot_w * k_dot_w * k_dot_w * k_dot_w;

	float w_length = glm::length(w);
	float L = w_length * w_length / g;
	float L2 = L * L;

	float damping = 0.001;
	float l2 = L2 * damping * damping;

	return A * exp(-1.0f / (k_length2 * L2)) / k_length4 * k_dot_w2 * exp(-k_length2 * l2);
}

complex Ocean::hTilde_0(int n_prime, int m_prime)
{
	float x1, x2, w;
	do {
		x1 = 2.f * (float)rand() / RAND_MAX - 1.f;
		x2 = 2.f * (float)rand() / RAND_MAX - 1.f;
		w = x1 * x1 + x2 * x2;
	} while (w >= 1.f);
	w = sqrt((-2.f * log(w)) / w);

	complex r(x1 * w, x2 * w);
	return r * sqrt(phillips(n_prime, m_prime) / 2.0f);
}
