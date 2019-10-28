#include "ChunkManager.h"
#include "util/MathUtils.h"
#include "util\Frustum.h"
#include <iostream>
#include <future>

const float LACUNARITY = 2.3f, GAIN = 0.48f, AMP_START = 1000.f, FREQ_START = 0.2f;
const int OCTAVES = 8;

const float MAP_WIDTH = 70811.136f;		// meters
const float MAP_HEIGHT = 48280.32f;
const float TERRAIN_MAX_HEIGHT = 1230.f;
const float MAP_SCALE = MAP_WIDTH / (Chunk::SIZE * 40);

ChunkManager::ChunkManager(const RenderSettings & renderSettings)
	: m_renderSettings(renderSettings),
	m_ocean(Chunk::SIZE)
{
	int radius = Chunk::SIZE / 2;
	int lodIndex = 0;
	for (const int &lod : TerrainConfig::LOD_STEP)
	{
		m_radiusToLOD[radius] = lodIndex;
		radius += Chunk::SIZE;
		lodIndex++;
	}
}

ChunkManager::~ChunkManager()
{
	for (auto &t : m_calcHeights) t.join();
	for (auto &t : m_calcMesh) t.join();
}

void ChunkManager::loadData(const std::string &filename)
{
	sf::Image image;
	image.loadFromFile(filename);
	m_imageWidth = image.getSize().x;
	m_imageHeight = image.getSize().y;
	for (int y = 0; y < m_imageHeight; y++)
		for (int x = 0; x < m_imageWidth; x++)
			m_imageData.push_back((int)image.getPixel(x, y).r);
}

void ChunkManager::update(const sf::Vector2i &flooredPos, const float &masterScale, const bool &reloadChunks)
{
	std::unique_lock<std::mutex> lock(m_mtx);
	if (true)
	{
		m_lastPos = flooredPos;
		m_lastScale = masterScale;

		int cZ = flooredPos.x, cX = flooredPos.y;

		bool changed = false;
		for (int i = 1; i <= m_chunkDist; i++)
		{
			for (int y = -i; y < i; y++)
				for (int x = -i; x < i; x++)
				{
					const sf::Vector2i pos =
						sf::Vector2i(cZ + x*Chunk::SIZE, cX + y*Chunk::SIZE);

					sf::Vector2i centeredPos = sf::Vector2i(x*Chunk::SIZE + Chunk::SIZE / 2, cX + y*Chunk::SIZE + Chunk::SIZE / 2);
					sf::Vector2i absPos(std::abs(centeredPos.x), std::abs(centeredPos.y));
					const int radius = std::max(absPos.x, absPos.y);

					bool create = false;
					{
						auto found = m_chunks.find(turnPositive(pos));
						if (found == m_chunks.end())
						{
							create = true;
							changed = true;
							m_chunks.emplace(std::piecewise_construct,
								std::forward_as_tuple(turnPositive(pos)),
								std::forward_as_tuple(pos, radiusToLODIndex(radius, m_lastScale), getOrientation(centeredPos), *this));
						}
						else if (reloadChunks)
							found->second.reload();
					}
				}
		}

		m_lastChanged = changed;

		m_chunkDist++;
		if (m_chunkDist >= *m_renderSettings.renderDist) m_chunkDist = 2;
	}

	bool threadSpaceAvailable = m_calcHeights.size() < 3;

	if (threadSpaceAvailable)
		if (m_heightQueue.size() != 0)
		{
			createMapThread(m_heightQueue.front());
			m_heightQueue.pop_front();
		}
	threadSpaceAvailable = m_calcMesh.size() < 3;

	if (threadSpaceAvailable)
		if (m_meshQueue.size() != 0)
		{
			createMeshThread(m_meshQueue.front());
			m_meshQueue.pop_front();
		}

	for (auto &c : m_heightMapCallbacks) if (getAt(c.position) != nullptr) c.callback(c.parameter);
	for (auto &c : m_meshCallbacks) if (getAt(c.position) != nullptr) c.callback(c.parameter);
	m_heightMapCallbacks.clear();
	m_meshCallbacks.clear();
}

void ChunkManager::updateWater(const float & deltaTime)
{
	m_ocean.update(deltaTime);
}

void ChunkManager::renderUpdate(const Frustum & frustum)
{
	m_drawedChunks.clear();

	std::unique_lock<std::mutex> lock(m_mtx);

	for (auto c = m_chunks.begin(); c != m_chunks.end();)
	{
		auto &chunk = c->second;
		const sf::Vector2i cPos = chunk.getPosition();
		if (!*m_renderSettings.freezeChunks && !withinRenderDistance(cPos))
		{
			chunk.clearData();
			c = m_chunks.erase(c);
		}
		else
		{
			sf::Vector2i centeredPos = cPos - m_lastPos + sf::Vector2i(Chunk::SIZE / 2, Chunk::SIZE / 2);
			sf::Vector2i absPos(std::abs(centeredPos.x), std::abs(centeredPos.y));

			if (!*m_renderSettings.freezeChunks) chunk.updateLOD(radiusToLODIndex(std::max(absPos.x, absPos.y), m_lastScale), getOrientation(centeredPos));
			//m_allTerrain.addMesh(chunk.getMesh());
			if (frustum.within(chunk))
			{
				m_drawedChunks.push_back(chunk);
				//m_drawedTerrain.addMesh(chunk.getMesh());
			}
			c++;
		}
	}
}

void ChunkManager::renderTerrain(const bool &onlyDrawed)
{
	if (onlyDrawed)
		for (auto &c : m_drawedChunks) c.get().renderTerrain();
	else
		for (auto &c : m_chunks) c.second.renderTerrain();
}

void ChunkManager::renderWater(const Ocean::ShaderInfo &shaderInfo, const bool &withOcean, const bool & onlyDrawed)
{
	if (onlyDrawed)
		for (auto &c : m_drawedChunks) m_ocean.renderAt(shaderInfo, c.get().getPosition(), withOcean);
	else
		for (auto &c : m_chunks) m_ocean.renderAt(shaderInfo, c.second.getPosition(), withOcean);
}

const bool ChunkManager::withinRenderDistance(const sf::Vector2i & chunkPos)
{
	const int maxRadius = *m_renderSettings.renderDist * Chunk::SIZE - Chunk::SIZE / 2;
	const sf::Vector2i relative = chunkPos + sf::Vector2i(Chunk::SIZE / 2, Chunk::SIZE / 2) - m_lastPos;
	return std::max(std::abs(relative.x), std::abs(relative.y)) <= maxRadius;
}

const float ChunkManager::getScale(const sf::Vector2i & pos, const float & max)
{
	auto centered = pos + sf::Vector2i(Chunk::SIZE / 2, Chunk::SIZE / 2);
	const float xDif = centered.x - m_lastPos.x;
	const float yDif = centered.y - m_lastPos.y;
	const float dif = std::max(std::abs(yDif), std::abs(xDif));
	const float scale = std::min(std::max(dif / max, 0.f), 1.f);
	return scale >= 0.9f ? 1.f : scale;		// close enough to 1 is good enough
}

const sf::Vector2i ChunkManager::turnPositive(const sf::Vector2i & pos)
{
	const int x = pos.x < 0 ? -pos.x * 2 - 1 : pos.x * 2;
	const int y = pos.y < 0 ? -pos.y * 2 - 1 : pos.y * 2;
	return{ x,y };
}

void ChunkManager::requestMapData(const MapData &_mapData)
{
	bool threadSpaceAvailable = m_calcHeights.size() < 3;

	auto insert = [&]() {
		bool replaced = false;
		std::replace_if(m_heightQueue.begin(), m_heightQueue.end(), [&](const MapData &m) {
			return replaced = m.config.position == _mapData.config.position;
		}, _mapData);
		if (!replaced) m_heightQueue.emplace_back(_mapData);
	};

	if (threadSpaceAvailable)
	{
		if (m_heightQueue.size() != 0)
		{
			insert();
			createMapThread(m_heightQueue.front());
			m_heightQueue.pop_front();
		}
		else
			createMapThread(_mapData);
	}
	else
		insert();
}

void ChunkManager::requestMeshData(const MeshData &_meshData)
{
	bool threadSpaceAvailable = m_calcMesh.size() < 3;

	auto insert = [&]() {

		bool replaced = false;
		std::replace_if(m_meshQueue.begin(), m_meshQueue.end(), [&](const MeshData &m) {
			return replaced = m.config.position == _meshData.config.position;
		}, _meshData);
		if (!replaced) m_meshQueue.emplace_back(_meshData);
	};

	if (threadSpaceAvailable)
	{
		if (m_meshQueue.size() != 0)
		{
			insert();
			createMeshThread(m_meshQueue.front());
			m_meshQueue.pop_front();
		}
		else
			createMeshThread(_meshData);
	}
	else
		insert();
}

const StitchEdge ChunkManager::getEdgesToStitch(const sf::Vector2i &location, const int &lod) const
{
	const Chunk *north = getAt(location + sf::Vector2i(0, Chunk::SIZE)),
		*south = getAt(location + sf::Vector2i(0, -Chunk::SIZE)),
		*east = getAt(location + sf::Vector2i(Chunk::SIZE, 0)),
		*west = getAt(location + sf::Vector2i(-Chunk::SIZE, 0));
	bool nStitch = north == nullptr ? false : north->getLOD() > lod,
		sStitch = south == nullptr ? false : south->getLOD() > lod,
		eStitch = east == nullptr ? false : east->getLOD() > lod,
		wStitch = west == nullptr ? false : west->getLOD() > lod;
	return{
		nStitch,
		eStitch,
		sStitch,
		wStitch
	};
}

const float ChunkManager::getMaxHeight(const sf::Vector2f & position) const
{
	sf::Vector2i pos(position);
	pos.x = std::floor((float)pos.x / Chunk::SIZE)*Chunk::SIZE;
	pos.y = std::floor((float)pos.y / Chunk::SIZE)*Chunk::SIZE;

	auto chunk = getAt(pos);

	if (chunk != nullptr)
	{
		return chunk->getMaxHeight(sf::Vector2i(position));
	}

	return 0.0f;
}

const Chunk * ChunkManager::getAt(const sf::Vector2i & location) const
{
	auto found = m_chunks.find(turnPositive(location));
	if (found != m_chunks.end()) return &found->second;
	return nullptr;
}

const int ChunkManager::radiusToLODIndex(const int & radius, const float &scale) const
{
	// TODO why are there spaces between large LOD??
	auto found = m_radiusToLOD.find(radius);
	if (found != m_radiusToLOD.end())
	{
		int s = (1.f - (scale <= 0.3 ? 0.f : scale)) / 0.1, i = 1;
		auto lastIndex = m_radiusToLOD.end();
		lastIndex--;
		while (found != lastIndex && i <= s)
		{
			found++;
			i++;
		}
		return found->second;
	}
	return m_radiusToLOD.rbegin()->second;
}

const StitchEdge ChunkManager::getOrientation(const sf::Vector2i & relative)
{
	const int x = relative.x, y = relative.y, ax = std::abs(x), ay = std::abs(y);
	const bool corner = ax == ay, vertical = ax < ay;
	if (x < 0)
	{
		if (y < 0)
		{
			if (corner) return BOTTOM_LEFT_CORNER;
			if (vertical) return BOTTOM_EDGE;
			return LEFT_EDGE;
		}
		if (corner) return TOP_LEFT_CORNER;
		if (vertical) return TOP_EDGE;
		return LEFT_EDGE;
	}

	if (y > 0)
	{
		if (corner) return TOP_RIGHT_CORNER;
		if (vertical) return TOP_EDGE;
		return RIGHT_EDGE;
	}
	if (corner) return BOTTOM_RIGHT_CORNER;
	if (vertical) return BOTTOM_EDGE;
	return RIGHT_EDGE;
}

void ChunkManager::createMapThread(const MapData & _mapData)
{
	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	m_calcHeights.emplace_back([=]() {
		typedef TerrainConfig tc;
		std::vector<float> heightMap(std::pow(tc::HEIGHT_MAP_DIMENSIONS_FULL + 1, 2));
		auto mapData = _mapData;

		auto index = [&](const int &x, const int &y) {
			return y*(tc::HEIGHT_MAP_DIMENSIONS_FULL + 1) + x;
		};

		auto getHeight = [&](const float &x, const float &y) {

			float value;
			if (m_imageData.size() != 0)
			{
				float xMeters = x * MAP_SCALE, yMeters = y * MAP_SCALE;
				xMeters += MAP_WIDTH / 2;
				yMeters += MAP_HEIGHT / 2;

				float xPixel = xMeters / MAP_WIDTH * m_imageWidth;
				float xPixel2 = xPixel + 1;
				float yPixel = m_imageHeight - yMeters / MAP_HEIGHT * m_imageHeight;
				float yPixel2 = yPixel + 1;

				xPixel = std::max(0.f, std::min((float)m_imageWidth, xPixel));
				yPixel = std::max(0.f, std::min((float)m_imageHeight, yPixel));
				xPixel2 = std::max(0.f, std::min((float)m_imageWidth, xPixel2));
				yPixel2 = std::max(0.f, std::min((float)m_imageHeight, yPixel2));

				float xA = xPixel - std::floor(xPixel);
				float yA = yPixel - std::floor(yPixel);

				int c1 = m_imageData[(int)yPixel * m_imageWidth + (int)xPixel];
				int c2 = m_imageData[(int)yPixel * m_imageWidth + (int)xPixel2];
				int c3 = m_imageData[(int)yPixel2 * m_imageWidth + (int)xPixel];
				int c4 = m_imageData[(int)yPixel2 * m_imageWidth + (int)xPixel2];

				auto lerp = [](const int &a, const int &b, const float &k) {
					if (a <= b) return glm::mix(a, b, k);
					return glm::mix(b, a, 1.f - k);
				};

				int color1 = lerp(c1, c2, xA);
				int color2 = lerp(c3, c4, xA);

				int color = lerp(color1, color2, yA);

				value = (float)color / 255 * (TERRAIN_MAX_HEIGHT) / MAP_SCALE;
				value = value == 0.f ? -10.f : value;
			}
			else
			{
				value = 200.f;
				float amplitude = AMP_START;
				float frequency = FREQ_START;

				sf::Vector2f st = { x / 400, y / 400 };
				for (int i = 0; i < OCTAVES; i++) {
					value += amplitude * mu::perlin(st * frequency);
					frequency *= LACUNARITY;
					amplitude *= GAIN;
				}

				value *= glm::mix(0.2f, 1.f, std::min(400.f, std::abs(value)) / 400.f);
			}

			return value;
		};

		int limit = 0;
		if (mapData.config.superLowLod) limit = tc::SUPER_LOW_LOD_STEP;

		for (int i = 0; i < tc::TOTAL_STEPS - limit; i++)
		{
			float xCoord = (float)mapData.config.position.x, yCoord = (float)mapData.config.position.y;

			int lod = tc::LOD_STEP[tc::TOTAL_STEPS - 1 - i];

			// CORNERS //

			float xLeft = xCoord - lod * mapData.config.heightIncr;
			float xRight = xCoord + (tc::HEIGHT_MAP_DIMENSIONS + lod) * mapData.config.heightIncr;
			float yBottom = yCoord - lod * mapData.config.heightIncr;
			float yTop = yCoord + (tc::HEIGHT_MAP_DIMENSIONS + lod) * mapData.config.heightIncr;

			heightMap[index(i, i)] = getHeight(xLeft, yBottom);
			heightMap[index(i, tc::HEIGHT_MAP_DIMENSIONS_FULL)] = getHeight(xLeft, yTop);
			heightMap[index(tc::HEIGHT_MAP_DIMENSIONS_FULL, tc::HEIGHT_MAP_DIMENSIONS_FULL)] = getHeight(xRight, yTop);
			heightMap[index(tc::HEIGHT_MAP_DIMENSIONS_FULL, i)] = getHeight(xRight, yBottom);

			// EDGES //

			for (int j = 0; j <= tc::HEIGHT_MAP_DIMENSIONS / lod; j++)
			{
				int offset = j * lod;
				heightMap[index(i, tc::TOTAL_STEPS + offset)] = getHeight(xLeft, yCoord + offset * mapData.config.heightIncr);
				heightMap[index(tc::HEIGHT_MAP_DIMENSIONS_FULL - i, tc::TOTAL_STEPS + offset)] = getHeight(xRight, yCoord + offset * mapData.config.heightIncr);
				heightMap[index(tc::TOTAL_STEPS + offset, i)] = getHeight(xCoord + offset * mapData.config.heightIncr, yBottom);
				heightMap[index(tc::TOTAL_STEPS + offset, tc::HEIGHT_MAP_DIMENSIONS_FULL - i)] = getHeight(xCoord + offset * mapData.config.heightIncr, yTop);
			}

			// FILL //
			for (int y = 0; y <= tc::HEIGHT_MAP_DIMENSIONS / lod; y++)
				for (int x = 0; x <= tc::HEIGHT_MAP_DIMENSIONS / lod; x++)
				{
					int X = x * lod, Y = y * lod;

					heightMap[index(tc::TOTAL_STEPS + X, tc::TOTAL_STEPS + Y)] = getHeight(xCoord + X * mapData.config.heightIncr, yCoord + Y * mapData.config.heightIncr);
				}
		}

		{
			std::unique_lock<std::mutex> lock(m_mtx);
			//if (mapData.config.superLowLod) std::cout << "\nFinished calculating for super low lod!";
			//std::cout << "\n" << config.cellSize;
			m_heightMapCallbacks.push_back({ mapData.callback, heightMap, mapData.config.position });
		}
		const auto id = std::this_thread::get_id();
		std::async([=]() {
			std::unique_lock<std::mutex> lock(m_mtx);
			auto found = std::find_if(m_calcHeights.begin(), m_calcHeights.end(), [&](const std::thread &t) { return t.get_id() == id; });
			if (found != m_calcHeights.end())
			{
				found->detach();
				m_calcHeights.erase(found);
			}
		});
	});
}

void ChunkManager::createMeshThread(const MeshData & _meshData)
{
	m_calcMesh.emplace_back([=]() {
		auto meshData = _meshData;
		TerrainMesh meshCopy = meshData.mesh;
		meshCopy.buildMesh(meshData.heightMap, meshData.config);

		{
			std::unique_lock<std::mutex> lock(m_mtx);
			m_meshCallbacks.push_back({ meshData.callback, meshCopy, meshData.config.position });
		}

		const auto id = std::this_thread::get_id();
		std::async([=]() {
			std::unique_lock<std::mutex> lock(m_mtx);
			auto found = std::find_if(m_calcMesh.begin(), m_calcMesh.end(), [&](const std::thread &t) { return t.get_id() == id; });
			if (found != m_calcMesh.end())
			{
				found->detach();
				m_calcMesh.erase(found);
			}
		});
	});
}
