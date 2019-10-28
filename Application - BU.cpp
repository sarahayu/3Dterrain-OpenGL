#include "Application.h"
#include <SFML\Graphics.hpp>
#include <iostream>
#include <cassert>
#include <array>
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "chunk\TerrainMesh.h"
#include "chunk\Chunk.h"
#include <fstream>

const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;

const int SCR_WIDTH = 1200, SCR_HEIGHT = 600;
const float FRAME_DURATION = 1.f / 60;

const glm::vec4 SKY(0.843137255, 0.8, 0.690196078, 1);
//const glm::vec4 WATER_COLOR(0.180392157, 0.509803922, 0.619607843, 0.8);
const glm::vec4 WATER_COLOR(0.0, 0.0, 0.0, 0.8);
glm::vec3 LIGHT_DIR(-1.0, -0.8, 1.0);

Application::Application()
	: 
	m_windowSize(SCR_WIDTH, SCR_HEIGHT),
	m_chunks({ &m_renderDistance, &m_renderDistPix, &m_freezeChunks }),
	m_refractionFBO(m_textures),
	m_reflectionFBO(m_textures),
	m_shadowFBO(m_textures),
	m_statGUI({ &m_renderStats, &m_ffps, &m_windowSize, &m_speed, &m_trueLocation, &m_freezeChunks })
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 0;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.sRgbCapable = true;
	m_window.create(sf::VideoMode(SCR_WIDTH, SCR_HEIGHT), "TerrainGen3D", sf::Style::Default, settings);
	LIGHT_DIR = glm::normalize(LIGHT_DIR);
	m_window.setMouseCursorVisible(false);
	m_lastMousePos = sf::Mouse::getPosition(m_window);
	sf::Mouse::setPosition({ SCR_WIDTH / 2, SCR_HEIGHT / 2 }, m_window);
	if (!gladLoadGL()) exit(-1);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	glEnable(GL_CULL_FACE);

	if (std::ifstream("resources/heightmap.png").good())
	{
		std::cout << "\nFound file! Loading from here...";
		m_fromImage = true;
		m_chunks.loadData("resources/heightmap.png");
	}

	m_terrainShader.load("resources/terrain-shader.vs", "resources/terrain-shader.fs");
	m_terrainShader.use();
	m_terrainShader.setVec3("lightColor", glm::vec3(1.0));
	m_terrainShader.setVec3("lightDirection", LIGHT_DIR);
	m_terrainShader.setVec4("fogColor", SKY);
	m_terrainShader.setFloat("texScale", 20.f);
	m_terrainShader.setFloat("viewDistance", m_renderDistPix);
	m_terrainShader.setVec4("waterColor", WATER_COLOR);
	m_terrainShader.setVec2("windowSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));

	m_waterPlaneShader.load("resources/water-shader.vs", "resources/water-shader.fs");
	m_waterPlaneShader.use();
	m_waterPlaneShader.setVec3("lightColor", glm::vec3(1.0, 0.99, 0.77));
	m_waterPlaneShader.setVec3("lightDirection", glm::vec3(-0.5, -1.0, -0.5));
	m_waterPlaneShader.setVec4("fogColor", SKY);
	m_waterPlaneShader.setFloat("texScale", 1.f);
	m_waterPlaneShader.setFloat("viewDistance", m_renderDistPix);
	m_waterPlaneShader.setVec4("waterColor", WATER_COLOR);

	m_skyboxShader.load("resources/skybox-shader.vs", "resources/skybox-shader.fs");
	m_skyboxShader.use();

	m_shadowShader.load("resources/shadow-shader.vs", "resources/shadow-shader.fs");
	m_shadowShader.use();

	m_trueLocation = glm::vec3(0.0, 400.0, 0.0);
	m_camera.Position = m_trueLocation;
	m_camera.turn(90.f, -89.f);

	m_terrainShader.use();
	m_terrainShader.setInt("sand",					m_textures.loadTex2D("resources/sand.jpg").offset);
	m_terrainShader.setInt("grass",					m_textures.loadTex2D("resources/grass.jpg").offset);
	m_terrainShader.setInt("limeStone",				m_textures.loadTex2D("resources/limeStone.jpg").offset);
	m_terrainShader.setInt("darkStone",				m_textures.loadTex2D("resources/darkStone.jpg").offset);
	m_terrainShader.setInt("snow",					m_textures.loadTex2D("resources/snow.jpg").offset);

	auto sand = m_textures.loadTex2D("resources/sand-normal-map.jpg");
	m_terrainShader.setInt("sandNormalMap",			sand.offset);
	m_terrainShader.setInt("rocksNormalMap",		m_textures.loadTex2D("resources/rock-normal-map.jpg").offset);
	m_terrainShader.setInt("stoneNormalMap",		m_textures.loadTex2D("resources/stone-normal-map.jpg").offset);
	m_terrainShader.setInt("raggedStoneNormalMap",	m_textures.loadTex2D("resources/raggedstone-normal-map.jpg").offset);
	m_terrainShader.setInt("water_normalmap",		m_textures.loadTex2D("resources/water-normal.jpg").offset);
	m_terrainShader.setInt("water_dudvmap", m_textures.loadTex2D("resources/water-dudv.jpg").offset);

	m_refractionFBO.initialize({ SCR_WIDTH, SCR_HEIGHT });
	m_terrainShader.setInt("refractionTex", m_refractionFBO.attachColorBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR }));
	m_terrainShader.setInt("depthTex", m_refractionFBO.attachDepthBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR }));
	m_refractionFBO.check();
	
	m_reflectionFBO.initialize({ SCR_WIDTH, SCR_HEIGHT });
	m_terrainShader.setInt("reflectionTex", m_reflectionFBO.attachColorBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR }));
	m_reflectionFBO.attachRenderBuffer();
	m_reflectionFBO.check();
	//m_terrainShader.setInt("reflectionTex", m_reflectTexIndex);


	m_skybox.initialize(std::vector<std::string> {
		"resources/right.jpg",
			"resources/left.jpg",
			"resources/top.jpg",
			"resources/bottom.jpg",
			"resources/front.jpg",
			"resources/back.jpg"
	});

	m_skyboxShader.use();
	m_skyboxShader.setInt("skybox", 0);

	m_shadowFBO.initialize({ SHADOW_WIDTH, SHADOW_HEIGHT });
	m_terrainShader.use();
	m_terrainShader.setInt("shadowMap", m_shadowFBO.attachDepthBuffer({ GL_CLAMP_TO_BORDER, GL_NEAREST }));
	
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	m_shadowFBO.check();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_modes.push_back({ "woAll", true, true, true });
	m_modes.push_back({ "normal" });/*
	m_modes.push_back({ "woWater", false, true, true });
	m_modes.push_back({ "woShadow", true });
	m_modes.push_back({ "woRefl", false, true });
	m_modes.push_back({ "woRefr", false, false, true });*/
}

Application::~Application()
{
}

void Application::run()
{
	sf::Clock clock;
	sf::Clock fpsClock;
	float dt = 0.f;

	while (m_window.isOpen())
	{
		const float elapsed = clock.restart().asSeconds();
		dt += elapsed;
		const bool updated = m_fps.update(dt);
		m_ffps = m_fps.getFPS();

		// Uncomment for stats
		/*
		if (updated && m_recording)
			if (!m_stats.addStat(m_modes[m_currentMode].mode, m_fps.getFPS()))
			{
				if (m_currentMode != (m_modes.size() - 1))
					std::cout << "\nChanging from " << m_modes[m_currentMode++].mode << " to " << m_modes[m_currentMode].mode;
				else
				{
					m_stats.printStats();
					m_recording = false;
				}
			}*/

		while (dt >= FRAME_DURATION)
		{
			dt -= FRAME_DURATION;

			input(dt);
			update(dt);
		}

		render();
	}
}

void Application::input(const float & deltaTime)
{
	sf::Event evnt;
	typedef sf::Keyboard sfk;
	while (m_window.pollEvent(evnt))
	{
		switch (evnt.type)
		{
		case sf::Event::Closed:
			m_window.close();
			break;
		case sf::Event::Resized:
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			unsigned int newWidth = evnt.size.width, newHeight = evnt.size.height;
			glViewport(0, 0, newWidth, newHeight);
			m_windowSize = sf::Vector2i(newWidth, newHeight);
			m_terrainShader.use();
			m_terrainShader.setVec2("windowSize", glm::vec2(evnt.size.width, newHeight));

			m_refractionFBO.initialize({ newWidth, newHeight });
			m_refractionFBO.attachColorBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR });
			m_refractionFBO.attachDepthBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR });
			m_refractionFBO.check();

			m_reflectionFBO.initialize({ newWidth, newHeight });
			m_reflectionFBO.attachColorBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR });
			m_reflectionFBO.attachRenderBuffer();
			m_reflectionFBO.check();
		}
			break;
		case sf::Event::MouseWheelMoved:
		{
			float y = evnt.mouseWheel.delta;
			m_speed = y < 0 ? m_speed / (std::abs(y)*1.1) : m_speed*y * 1.1;
			m_speed = std::min(4000.f, std::max(5.f, m_speed));
		}
		break;
		case sf::Event::KeyPressed:
			if (m_focused)
				switch (evnt.key.code)
				{
				case sfk::P:
					m_recording = true;
					std::cout << "\nRecording stats...";
					break;
				case sfk::R:
					m_reloadChunks = true;
					std::cout << "\nReloading chunks...";
					break;
				case sfk::Z:
					if (m_currentMode < m_modes.size() - 1) m_currentMode++;
					else m_currentMode = 0;
					break;
				case sfk::F:
					m_freezeChunks = !m_freezeChunks;
					break;
				case sfk::M:
					m_wireFrame = !m_wireFrame;
					break;
				case sfk::F3:
					m_renderStats = !m_renderStats;
					break;
				}
			//if (evnt.key.code == sf::Keyboard::M) m_debug = !m_debug;
			break;
		case sf::Event::LostFocus:
			m_focused = false;
			break;
		case sf::Event::GainedFocus:
			m_focused = true;
			break;
		default:
			break;
		}
	}

	if (m_focused)
	{
		sf::Vector2i mouseMovement = sf::Mouse::getPosition(m_window) - m_lastMousePos;
		sf::Mouse::setPosition({ m_windowSize.x / 2, m_windowSize.y / 2 }, m_window);
		m_lastMousePos = sf::Mouse::getPosition(m_window);

		m_camera.ProcessMouseMovement(mouseMovement.x, -mouseMovement.y);

		sf::Vector2f movement;

		auto toVector = [&](const float &yaw) {
			return sf::Vector2f(std::sin(glm::radians(yaw)), std::cos(glm::radians(yaw)));
		};

		if (sfk::isKeyPressed(sfk::W)) movement += toVector(m_camera.Yaw);
		if (sfk::isKeyPressed(sfk::S)) movement += toVector(m_camera.Yaw + 180.f);
		if (sfk::isKeyPressed(sfk::A)) movement += toVector(m_camera.Yaw - 90.f);
		if (sfk::isKeyPressed(sfk::D)) movement += toVector(m_camera.Yaw + 90.f);

		float yMov = 0.f;
		if (sfk::isKeyPressed(sfk::LShift)) yMov -= 1.f;
		if (sfk::isKeyPressed(sfk::Space)) yMov += 1.f;

		m_camera.Position.y += yMov * m_speed * FRAME_DURATION;

		m_trueLocation.x += movement.y * m_speed * FRAME_DURATION;
		m_trueLocation.z += movement.x * m_speed * FRAME_DURATION;

		float maxHeight = std::max(0.f, m_chunks.getMaxHeight({ m_trueLocation.z, m_trueLocation.x })) + 5.f;
		if (m_camera.Position.y < maxHeight) m_camera.Position.y = maxHeight;
		m_trueLocation.y = m_camera.Position.y / m_masterScale;
		m_masterScale = std::min(1.f, std::pow(1000.f / m_camera.Position.y, 2));
		m_renderDistance = std::min(20, (int)(4 + (1.f - (m_masterScale <= 0.3 ? 0.f : m_masterScale)) / 0.1 * 2));
		m_renderDistPix = m_renderDistance * Chunk::SIZE;

		m_camera.Position.x = m_trueLocation.x * m_masterScale;
		m_camera.Position.z = m_trueLocation.z * m_masterScale;


	}
}

void Application::update(const float &deltaTime)
{
	if (!m_freezeChunks)
	{
		int cZ = std::round(m_trueLocation.z / Chunk::SIZE)*Chunk::SIZE;
		int cX = std::round(m_trueLocation.x / Chunk::SIZE)*Chunk::SIZE;

		m_chunks.update({ cZ, cX }, m_masterScale, m_reloadChunks);
		m_reloadChunks = false;
	}
}

void Application::render()
{
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	view = m_camera.getViewMatrix();
	model = glm::scale(model, glm::vec3(m_masterScale));
	projection = glm::perspective(glm::radians(90.f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 3000.0f);
	//glm::mat4 projection2 = glm::perspective(glm::radians(30.f), (float)SCR_WIDTH / SCR_HEIGHT, 50.f, 2000.0f);


	view = m_camera.getViewMatrix();

	glm::mat4 projView = projection * view;
	Frustum frustum(projView * model);

	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	float range = m_renderDistPix;
	lightProjection = glm::ortho(-range, range, -range, range, 0.0f, 6000.f);
	glm::vec3 level0(m_trueLocation.x, 0.f, m_trueLocation.z);
	lightView = glm::lookAt(-LIGHT_DIR * (float)m_renderDistPix + level0,
		level0,
		glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view

	m_chunks.renderUpdate(frustum);

	m_textures.activateTextures();

	const auto &mode = m_modes[m_currentMode];

	if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (!mode.withoutShadow)
	{
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		m_shadowFBO.bindFBO();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		m_shadowShader.use();
		m_shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		m_shadowShader.setMat4("model", model);
		m_chunks.renderTerrain(false);
	}

	view = m_camera.getViewMatrix();
	m_terrainShader.use();
	m_terrainShader.setMat4("model", model);
	m_terrainShader.setMat4("view", view);
	m_terrainShader.setMat4("projection", projection);
	m_terrainShader.setVec3("viewPos", m_camera.Position);
	m_terrainShader.setFloat("viewDistance", 4 * Chunk::SIZE + (1.f - m_masterScale) * 20 * Chunk::SIZE);
	m_terrainShader.setFloat("renderMode", 1);
	m_terrainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, m_windowSize.x, m_windowSize.y);
	if (!mode.withoutRefraction)
	{
		m_refractionFBO.bindFBO();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);

		m_chunks.renderTerrain();
	}

	if (!mode.withoutReflection)
	{
		m_reflectionFBO.bindFBO();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glDepthMask(false);
		glCullFace(GL_BACK);
		m_waterPlaneShader.use();
		m_waterPlaneShader.setMat4("model", model);
		m_waterPlaneShader.setMat4("view", view);
		m_waterPlaneShader.setMat4("projection", projection);
		m_waterPlaneShader.setVec3("viewPos", m_camera.Position);
		m_waterPlaneShader.setFloat("viewDistance", 4 * Chunk::SIZE + (1.f - m_masterScale) * 10 * Chunk::SIZE);
		m_waterPlaneShader.setFloat("timeOffset", m_clock.getElapsedTime().asSeconds() / 1000);
		m_waterPlaneShader.setBool("reflection", true);

		m_chunks.renderWater();

		glDepthMask(true);
		glCullFace(GL_FRONT);

		m_terrainShader.use();
		m_terrainShader.setFloat("renderMode", 0);
		m_terrainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		m_chunks.renderTerrain();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	m_skyboxShader.use();
	view = glm::mat4(glm::mat3(m_camera.getViewMatrix())); // remove translation from the view matrix
	m_skyboxShader.setMat4("view", view);
	m_skyboxShader.setMat4("projection", projection);
	// skybox cube
	m_skybox.draw();
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
						  //glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);

	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)

	glEnable(GL_DEPTH_TEST);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(true);


	glCullFace(GL_BACK);
	if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	m_textures.activateTextures();
	view = m_camera.getViewMatrix();
	m_terrainShader.use();
	m_terrainShader.setMat4("view", view);
	m_terrainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	m_terrainShader.setFloat("renderMode", 2);
	m_terrainShader.setMat4("model", model);
	m_terrainShader.setMat4("view", view);
	m_terrainShader.setMat4("projection", projection);
	m_terrainShader.setVec3("viewPos", m_camera.Position);
	m_terrainShader.setBool("withoutReflection", mode.withoutReflection);
	m_terrainShader.setBool("withoutRefraction", mode.withoutRefraction);
	m_terrainShader.setBool("withoutShadow", mode.withoutShadow);
	float elapsed = m_clock.getElapsedTime().asSeconds();
	m_terrainShader.setFloat("time", elapsed / 100);
	m_chunks.renderTerrain();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	m_window.pushGLStates();
	m_window.resetGLStates();

	m_statGUI.render(m_window);

	m_window.popGLStates();

	m_window.display();
}