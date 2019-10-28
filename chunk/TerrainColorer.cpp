#include "TerrainColorer.h"
#include "TerrainMesh.h"
#include "../util/MathUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const glm::vec4 SAND(1.0, 0.0, 0.0, 0.0);
const glm::vec4 GRASS(0.0, 1.0, 0.0, 0.0);
const glm::vec4 STONE(0.0, 0.0, 1.0, 0.0);
const glm::vec4 SNOW(0.0, 0.0, 0.0, 1.0);

const float SNOW_LEVEL = 500.f;
const float SEA_LEVEL = 5.f;

const double SNOW_MAX_ANGLE = 0.423, GRASS_MAX_ANGLE = 0.342;

void TerrainColorer::colorize(const TerrainConfig & config, TerrainMesh & mesh)
{
	for (int y0 = 0; y0 <= config.dimensions - 1; y0++)
		for (int x0 = 0; x0 <= config.dimensions - 1; x0++)
		{
			int x1 = x0 + 1, y1 = y0 + 1;
			int c1 = (y0 * (config.dimensions)+x0) * 3 * 4;
			int c2 = c1 + 3;
			int c3 = c2 + 3;
			int c4 = c3 + 3;

			auto calcColor = [&](const int &index) {
				glm::vec3 vertLoc(mesh.m_vertices[index],
					mesh.m_vertices[index + 1],
					mesh.m_vertices[index + 2]);
				glm::vec3 norm(mesh.m_normals[index],
					mesh.m_normals[index + 1],
					mesh.m_normals[index + 2]);

				glm::vec4 landColor = SAND;
				if (vertLoc.y >= SNOW_LEVEL)
				{
					landColor = glm::mix(STONE, SNOW, norm.y);
					//if (mu::noise({ (int)vertLoc.z, (int)vertLoc.x }).x <= std::min(200.f, vertLoc.y - SNOW_LEVEL) / 200.f)
					//{
					//	landColor = glm::mix(STONE, SNOW, norm.y);
					//	/*if (norm.y > 0.423) landColor = SNOW;
					//	else landColor = STONE;*/
					//}
					//else
					//{
					//	landColor = glm::mix(STONE, GRASS, norm.y);
					//	/*if (norm.y > 0.342) landColor = GRASS;
					//	else landColor = STONE;*/
					//}
				}
				else if (vertLoc.y > SEA_LEVEL + 2.0)
				{
					landColor = glm::mix(STONE, GRASS, std::min(1.f, norm.y + 0.2f));
					/*if (norm.y > 0.342) landColor = GRASS;
					else landColor = STONE;*/
				}

				mesh.m_colors.insert(mesh.m_colors.end(), { landColor.x, landColor.y, landColor.z, landColor.a });
			};

			calcColor(c1);
			calcColor(c2);
			calcColor(c3);
			calcColor(c4);
		}
}
