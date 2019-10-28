#pragma once

struct TerrainConfig;
struct TerrainMesh;

struct TerrainColorer
{
	static void colorize(const TerrainConfig &config, TerrainMesh &mesh);
};