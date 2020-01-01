#pragma once
#include <SFML\Graphics.hpp>
#include <mutex>
#include <thread>
#include <atomic>
#include <list>
#include <set>
#include "chunk\Chunk.h"
#include "fft-ocean\Ocean.h"

class Frustum;
class Shader;

class ChunkManager
{
public:
	struct RenderSettings
	{
		int *renderDist;
		int *renderDistPix;
		bool *freezeChunks;
	};

	ChunkManager(const RenderSettings &renderSettings);
	~ChunkManager();
	void loadData(const std::string &filename);
	void update(const sf::Vector2i &flooredPos, const float &masterScale, const bool &reloadChunks);
	void updateWater(const float &deltaTime);
	void renderUpdate(const Frustum &frustum);
	void renderTerrain(const bool &onlyDrawed = true);
	void renderWater(const Ocean::ShaderInfo &shaderInfo, const bool &withOcean, const bool &onlyDrawed = true);
	struct MapData
	{
		std::function<void(const std::vector<float>&)> callback;
		TerrainConfig config;
	};
	void requestMapData(const MapData &mapData);		// TODO move to some worldgen class
	struct MeshData
	{
		std::function<void(const TerrainMesh&)> callback;
		TerrainConfig config;
		std::vector<float> heightMap;
		TerrainMesh mesh;
	};
	void requestMeshData(const MeshData &meshData);		// TODO move to some worldgen class
	const StitchEdge getEdgesToStitch(const sf::Vector2i &location, const int &lod) const;

	const float getMaxHeight(const sf::Vector2f &position) const;
private:
	std::vector<unsigned int> m_imageData;
	int m_imageWidth, m_imageHeight;

	const bool withinRenderDistance(const sf::Vector2i &chunkPos);
	const float getScale(const sf::Vector2i &pos, const float &max);

	static const sf::Vector2i turnPositive(const sf::Vector2i &pos);
	const Chunk* getAt(const sf::Vector2i &location) const;

	const int radiusToLODIndex(const int &radius, const float &scale) const;
	static const StitchEdge getOrientation(const sf::Vector2i &relative);

	void createMapThread(const MapData &mapData);
	void createMeshThread(const MeshData &meshData);

	RenderSettings m_renderSettings;
	sf::Vector2i m_lastPos;
	float m_lastScale = 0.f;
	bool m_lastChanged = false;

	sf::Clock m_clock;
/*
	Model m_drawedTerrain;
	Model m_allTerrain;*/

	struct Compare2D
	{
		bool operator()(const sf::Vector2i& a, const sf::Vector2i& b) const {
			return a.y < b.y
				|| (a.y == b.y
					&& a.x < b.x);
		}
	};

	std::map<sf::Vector2i, Chunk, Compare2D> m_chunks;
	std::vector<std::reference_wrapper<Chunk>> m_drawedChunks;
	int m_chunkDist = 1;

	Ocean m_ocean;

	std::map<int, int> m_radiusToLOD;

	template <class T>
	struct Callback
	{
		std::function<void(const T&) > callback;
		T parameter;
		sf::Vector2i position;
	};
	std::vector<Callback<std::vector<float>>> m_heightMapCallbacks;
	std::vector<Callback<TerrainMesh>>m_meshCallbacks;
	std::list<MapData> m_heightQueue;
	std::list<MeshData> m_meshQueue;
	std::vector<std::thread> m_calcHeights;
	std::vector<std::thread> m_calcMesh;
	std::mutex m_mtx;

};