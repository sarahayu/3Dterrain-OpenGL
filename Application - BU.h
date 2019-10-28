#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <SFML\Graphics.hpp>
#include <queue>
#include <set>
#include <list>
#include "Camera.h"
#include "Shader.h"
#include "Textures.h"
#include "FBO.h"
#include "Skybox.h"
#include "util/FPS.h"
#include "util/Frustum.h"
#include "util/StatUtils.h"
#include "ChunkManager.h"
#include "GUIStats.h"

class TerrainMesh;

class Application
{
public:
	Application();
	~Application();

	void run();

private:

	void input(const float &deltaTime);
	void update(const float &deltaTime);
	void render();

	sf::RenderWindow m_window;
	sf::Vector2i m_windowSize;
	
	int m_renderDistance = 4;
	int m_renderDistPix = m_renderDistance * Chunk::SIZE;
	bool m_fromImage = false;

	bool m_freezeChunks = false;
	ChunkManager m_chunks;

	Camera m_camera;
	float m_lastX, m_lastY;
	float m_speed = 10.f;
	float m_masterScale = 1.f;
	sf::Vector2i m_lastMousePos;
	glm::vec3 m_trueLocation;

	Shader m_terrainShader;
	Shader m_waterPlaneShader;		// need this for sky reflection
	Shader m_skyboxShader;
	Shader m_shadowShader;

	FPS m_fps;
	float m_ffps;
	GUIStats m_statGUI;
	bool m_renderStats = true;
	bool m_debug = true;
	bool m_focused = true;
	bool m_reloadChunks = false;
	bool m_wireFrame = false;

	bool m_recording = false;
	StatUtils m_stats;
	struct StatMode
	{
		std::string mode;
		bool withoutShadow,
			withoutReflection,
			withoutRefraction;
	};
	std::vector<StatMode> m_modes;
	int m_currentMode = 0;

	sf::Clock m_clock;

	Textures m_textures;

	FBO m_refractionFBO;
	FBO m_reflectionFBO;
	FBO m_shadowFBO;

	Skybox m_skybox;
};