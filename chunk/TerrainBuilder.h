#pragma once
#include <vector>

struct TerrainConfig;
class TerrainMesh;

struct TerrainBuilder
{
	static void buildMesh(const TerrainConfig &config, const std::vector<float> &heightMap, TerrainMesh &mesh);
};