#include "TerrainBuilder.h"
#include "TerrainMesh.h"
#include <iostream>


void TerrainBuilder::buildMesh(const TerrainConfig & config, const std::vector<float> &heightMap, TerrainMesh & mesh)
{
	typedef TerrainConfig tc;

	for (int y = 0; y <= config.fullDimensions; y++)
		for (int x = 0; x <= config.fullDimensions; x++)
		{
			const bool notEdge = x != 0 && x != config.fullDimensions && y != 0 && y != config.fullDimensions;

			auto index = [&](const int &X, const int &Y) {
				int xOffset, yOffset;
				if (X == 0)
					xOffset = tc::TOTAL_STEPS - config.lodIndex - 1;
				else if (X == config.fullDimensions)
					xOffset = tc::HEIGHT_MAP_DIMENSIONS + 1 + tc::TOTAL_STEPS + config.lodIndex + 1 - 1;
				else
					xOffset = tc::TOTAL_STEPS + (X - 1)*config.lodStep;
				if (Y == 0)
					yOffset = tc::TOTAL_STEPS - config.lodIndex - 1;
				else if (Y == config.fullDimensions)
					yOffset = tc::HEIGHT_MAP_DIMENSIONS + 1 + tc::TOTAL_STEPS + config.lodIndex + 1 - 1;
				else
					yOffset = tc::TOTAL_STEPS + (Y - 1)*config.lodStep;
				return yOffset * (tc::HEIGHT_MAP_DIMENSIONS_FULL + 1) + xOffset;
			};

			const float xCoord = (float)config.position.x + (x - 1) * config.cellSize;
			const float yCoord = (float)config.position.y + (y - 1) * config.cellSize;

			float value = heightMap[index(x, y)];

			mesh.m_verticesFull.push_back({ yCoord, value, xCoord });
			mesh.m_texCoordsFull.push_back({ (float)(x - 1) / config.dimensions, (float)(y - 1) / config.dimensions });

			if (notEdge)
			{
				mesh.m_vertices.push_back({ yCoord, value, xCoord });
				mesh.m_texCoords.push_back({ (float)(x - 1) / config.dimensions, (float)(y - 1) / config.dimensions });
			}
		}
}
