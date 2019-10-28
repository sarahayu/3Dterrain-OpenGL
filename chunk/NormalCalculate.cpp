#include "NormalCalculate.h"
#include "TerrainMesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

void NormalCalculate::calculate(const TerrainConfig & config, TerrainMesh & mesh)
{
	std::vector<glm::vec3> faceNormalsFull;
	std::vector<glm::vec3> faceTangentsFull;

	for (int y0 = 0; y0 < config.fullDimensions; y0++)
		for (int x0 = 0; x0 < config.fullDimensions; x0++)
		{
			int c1 = (y0*(config.fullDimensions + 1)+x0),
				c2 = c1 + config.fullDimensions + 1,
				c3 = c1 + config.fullDimensions + 1 + 1,
				c4 = c1 + 1;

			glm::vec3 pos1(mesh.m_verticesFull[c1].x, mesh.m_verticesFull[c1].y, mesh.m_verticesFull[c1].z);
			glm::vec3 pos2(mesh.m_verticesFull[c2].x, mesh.m_verticesFull[c2].y, mesh.m_verticesFull[c2].z);
			glm::vec3 pos3(mesh.m_verticesFull[c3].x, mesh.m_verticesFull[c3].y, mesh.m_verticesFull[c3].z);
			glm::vec3 pos4(mesh.m_verticesFull[c4].x, mesh.m_verticesFull[c4].y, mesh.m_verticesFull[c4].z);

			int cc1 = (y0*(config.fullDimensions + 1) + x0),
				cc2 = cc1 + config.fullDimensions + 1,
				cc3 = cc1 + config.fullDimensions + 1 + 1,
				cc4 = cc1 + 1;

			glm::vec2 uv1(mesh.m_texCoordsFull[cc1].x, mesh.m_texCoordsFull[cc1].y);
			glm::vec2 uv2(mesh.m_texCoordsFull[cc2].x, mesh.m_texCoordsFull[cc2].y);
			glm::vec2 uv3(mesh.m_texCoordsFull[cc3].x, mesh.m_texCoordsFull[cc3].y);
			glm::vec2 uv4(mesh.m_texCoordsFull[cc4].x, mesh.m_texCoordsFull[cc4].y);

			glm::vec3 edge1 = pos2 - pos1;
			glm::vec3 edge2 = pos3 - pos1;
			glm::vec3 edge3 = pos4 - pos1;
			glm::vec2 deltaUV1 = uv2 - uv1;
			glm::vec2 deltaUV2 = uv3 - uv1;
			glm::vec2 deltaUV3 = uv4 - uv1;

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			glm::vec3 tangent;
			tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			tangent = glm::normalize(tangent);

			glm::vec3 normal = glm::cross(edge1, edge2);
			if (normal.y < 0) normal = -normal;

			faceNormalsFull.push_back(normal);
			faceTangentsFull.push_back(tangent);

			f = 1.0f / (deltaUV2.x * deltaUV3.y - deltaUV3.x * deltaUV2.y);
			tangent.x = f * (deltaUV3.y * edge2.x - deltaUV2.y * edge3.x);
			tangent.y = f * (deltaUV3.y * edge2.y - deltaUV2.y * edge3.y);
			tangent.z = f * (deltaUV3.y * edge2.z - deltaUV2.y * edge3.z);
			tangent = glm::normalize(tangent);

			normal = glm::cross(edge3, edge2);
			if (normal.y < 0) normal = -normal;

			faceNormalsFull.push_back(normal);
			faceTangentsFull.push_back(tangent);
		}

	for (int y = 1; y <= config.fullDimensions - 1; y++)
	{
		for (int x = 1; x <= config.fullDimensions - 1; x++)
		{
			int xx = x * 2;
			sf::Vector2i t1(xx - 1, y), t2(xx, y), t3(xx + 1, y),
				t4(xx - 2, y - 1), t5(xx - 1, y - 1), t6(xx, y - 1);

			glm::vec3 normal = (faceNormalsFull[t1.y * (config.fullDimensions * 2) + t1.x]
				+ faceNormalsFull[t2.y * (config.fullDimensions * 2) + t2.x]
				+ faceNormalsFull[t3.y * (config.fullDimensions * 2) + t3.x]
				+ faceNormalsFull[t4.y * (config.fullDimensions * 2) + t4.x]
				+ faceNormalsFull[t5.y * (config.fullDimensions * 2) + t5.x]
				+ faceNormalsFull[t6.y * (config.fullDimensions * 2) + t6.x]);

			normal.x /= 6; normal.y /= 6; normal.z /= 6;

			normal = glm::normalize(normal);

			mesh.m_normals.push_back({ normal.x,normal.y,normal.z });

			glm::vec3 tangent = (faceTangentsFull[t1.y * (config.fullDimensions * 2) + t1.x]
				+ faceTangentsFull[t2.y * (config.fullDimensions * 2) + t2.x]
				+ faceTangentsFull[t3.y * (config.fullDimensions * 2) + t3.x]
				+ faceTangentsFull[t4.y * (config.fullDimensions * 2) + t4.x]
				+ faceTangentsFull[t5.y * (config.fullDimensions * 2) + t5.x]
				+ faceTangentsFull[t6.y * (config.fullDimensions * 2) + t6.x]);

			tangent.x /= 6; tangent.y /= 6; tangent.z /= 6;

			tangent = glm::normalize(tangent);

			mesh.m_tangents.push_back({ tangent.x,tangent.y,tangent.z });
		}
	}
}
