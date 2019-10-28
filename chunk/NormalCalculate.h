#pragma once

struct TerrainConfig;
class TerrainMesh;

struct NormalCalculate
{
	static void calculate(const TerrainConfig &config, TerrainMesh &mesh);
};