#pragma once
#include <vector>
#include <array>

struct Mesh;

class Model
{
public:
	
	void addMesh(const Mesh &mesh);

	void render();
	void clear();

private:
	unsigned int m_vao;
	unsigned int m_indiceCount = 0;
	std::array<unsigned int, 5> m_vbos;
	bool m_buffered = false;
	std::vector<unsigned int> m_ebos;

	std::vector<float> m_vertices;
	std::vector<float> m_normals;
	std::vector<float> m_tangents;
	std::vector<float> m_texCoords;

	std::vector<unsigned int> m_indices;
};

struct Mesh
{
	std::vector<float> *vertices;
	std::vector<float> *normals;
	std::vector<float> *tangents;
	std::vector<float> *texCoords;

	std::vector<unsigned int> *indices;
};