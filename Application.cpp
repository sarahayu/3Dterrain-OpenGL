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
//const glm::vec4 SKY(0.0, 0.0, 0.0, 1);
glm::vec3 LIGHT_DIR(-1.0, -0.8, 1.0);
const float REFLECTION = 0.f, REFRACTION = 1.f, FINAL_PASS = 2.f;

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
	m_window.setMouseCursorVisible(false);
	LIGHT_DIR = glm::normalize(LIGHT_DIR);
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

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	if (std::ifstream("resources/heightmap.png").good())
	{
		std::cout << "\nFound file! Loading from here...";
		m_fromImage = true;
		m_chunks.loadData("resources/heightmap.png");
	}

	m_skybox.initialize(std::vector<std::string> {
		"resources/right.jpg",
			"resources/left.jpg",
			"resources/top.jpg",
			"resources/bottom.jpg",
			"resources/front.jpg",
			"resources/back.jpg"
	});

	m_trueLocation = glm::vec3(0.0, 400.0, 0.0);
	m_camera.Position = m_trueLocation;
	m_camera.turn(90.f, -89.f);
	
	// --------------------- Initialize some FBOs and texture indices ----------------- //
	unsigned int shadowTex, refractionTex, depthTex, reflectionTex;

	m_shadowFBO.initialize({ SHADOW_WIDTH, SHADOW_HEIGHT });
	shadowTex = m_shadowFBO.attachDepthBuffer({ GL_CLAMP_TO_BORDER, GL_NEAREST });
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	m_shadowFBO.check();

	m_refractionFBO.initialize({ SCR_WIDTH, SCR_HEIGHT });
	refractionTex = m_refractionFBO.attachColorBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR });
	depthTex = m_refractionFBO.attachDepthBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR });
	m_refractionFBO.check();
	
	m_reflectionFBO.initialize({ SCR_WIDTH, SCR_HEIGHT });
	reflectionTex = m_reflectionFBO.attachColorBuffer({ GL_CLAMP_TO_EDGE, GL_LINEAR });
	m_reflectionFBO.attachRenderBuffer();
	m_reflectionFBO.check();
	// ------------------------------------------------------------------------------- //

	// --------------------------- Load shaders and set uniforms --------------------- //
	m_terrainShader.load("resources/terrain-shader.vs", "resources/terrain-shader.fs");
	m_waterPlaneShader.load("resources/water-shader.vs", "resources/water-shader.fs");
	m_skyboxShader.load("resources/skybox-shader.vs", "resources/skybox-shader.fs");
	m_shadowShader.load("resources/shadow-shader.vs", "resources/shadow-shader.fs");
	m_reflectionShader.load("resources/reflection-shader.vs", "resources/reflection-shader.fs");
	
	const unsigned int waterNormal = m_textures.loadTex2D("resources/water-normal.jpg").offset,
		waterDUDV = m_textures.loadTex2D("resources/water-dudv.jpg").offset;

	m_terrainShader.use();
	m_terrainShader.setVec3		("lightColor",				glm::vec3(1.0));
	m_terrainShader.setVec3		("lightDirection",			LIGHT_DIR);
	m_terrainShader.setVec4		("fogColor",				SKY);
	m_terrainShader.setFloat	("texScale",				20.f);
	m_terrainShader.setFloat	("viewDistance",			m_renderDistPix);
	m_terrainShader.setFloat	("scale",					m_masterScale);
	m_terrainShader.setInt		("sand",					m_textures.loadTex2D("resources/sand.jpg").offset);
	m_terrainShader.setInt		("grass",					m_textures.loadTex2D("resources/grass.jpg").offset);
	m_terrainShader.setInt		("limeStone",				m_textures.loadTex2D("resources/limestone.jpg").offset);
	m_terrainShader.setInt		("darkStone",				m_textures.loadTex2D("resources/darkstone.jpg").offset);
	m_terrainShader.setInt		("snow",					m_textures.loadTex2D("resources/snow.jpg").offset);
	m_terrainShader.setInt		("sandNormalMap",			m_textures.loadTex2D("resources/sand-normal-map.jpg").offset);
	m_terrainShader.setInt		("rocksNormalMap",			m_textures.loadTex2D("resources/rock-normal-map.jpg").offset);
	m_terrainShader.setInt		("stoneNormalMap",			m_textures.loadTex2D("resources/stone-normal-map.jpg").offset);
	m_terrainShader.setInt		("raggedStoneNormalMap",	m_textures.loadTex2D("resources/raggedstone-normal-map.jpg").offset);
	m_terrainShader.setInt		("shadowMap",				shadowTex);

	m_waterPlaneShader.use();
	m_waterPlaneShader.setVec4	("fogColor",				SKY);
	m_waterPlaneShader.setFloat	("viewDistance",			m_renderDistPix);
	m_waterPlaneShader.setFloat	("scale",					m_masterScale);
	m_waterPlaneShader.setInt	("water_normalmap",			waterNormal);
	m_waterPlaneShader.setInt	("water_dudvmap",			waterDUDV);
	m_waterPlaneShader.setVec2	("windowSize",				glm::vec2(SCR_WIDTH, SCR_HEIGHT));

	m_skyboxShader.use();
	m_skyboxShader.setInt		("skybox",					0);	

	m_reflectionShader.use();
	m_reflectionShader.setVec3	("lightColor",				glm::vec3(1.0));
	m_reflectionShader.setVec3	("lightDirection",			LIGHT_DIR);
	m_reflectionShader.setVec4	("fogColor",				SKY);
	m_reflectionShader.setFloat	("viewDistance",			m_renderDistPix);
	m_reflectionShader.setFloat	("scale",					m_masterScale);
	m_reflectionShader.setVec2	("windowSize",				glm::vec2(SCR_WIDTH, SCR_HEIGHT));
	m_reflectionShader.setInt	("refractionTex",			refractionTex);
	m_reflectionShader.setInt	("depthTex",				depthTex);
	m_reflectionShader.setInt	("reflectionTex",			reflectionTex);
	m_reflectionShader.setInt	("water_normalmap",			waterNormal);
	m_reflectionShader.setInt	("water_dudvmap",			waterDUDV);
	m_reflectionShader.setInt	("shadowMap",				shadowTex);
	// ---------------------------------------------------------------------------- //

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_modes.push_back({ "woAll", true, true, true });
	m_modes.push_back({ "normal" });
	m_modes.push_back({ "ocean", false, true, true, true });
	m_withOcean = m_modes[m_currentMode].withOcean;
	/*
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
			m_reflectionShader.use();
			m_reflectionShader.setVec2("windowSize", glm::vec2(evnt.size.width, newHeight));
			m_waterPlaneShader.use();
			m_waterPlaneShader.setVec2("windowSize", glm::vec2(evnt.size.width, newHeight));

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
					m_currentMode = (++m_currentMode) % m_modes.size();
					m_withOcean = m_modes[m_currentMode].withOcean;
					break;
				case sfk::F:
					m_freezeChunks = !m_freezeChunks;
					break;
				case sfk::M:
					m_wireFrame = !m_wireFrame;
					break;
				case sfk::O:
					m_wideView = !m_wideView;
					break;
				case sfk::F3:
					m_renderStats = !m_renderStats;
					break;
				}
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
		sf::Mouse::setPosition(m_windowSize / 2, m_window);
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

		m_mvp.view = m_camera.getViewMatrix();
		m_mvp.model = glm::scale(glm::mat4(1.0), glm::vec3(m_masterScale));
		m_mvp.projection = glm::perspective(m_wideView ? glm::radians(90.f) : glm::radians(45.f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 3000.0f);

		Frustum frustum(m_mvp.mvp());
		m_chunks.renderUpdate(frustum);
	}
}

void Application::update(const float &deltaTime)
{
	if (m_modes[m_currentMode].withOcean) m_chunks.updateWater(deltaTime);

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
	glm::mat4 lightSpaceMatrix;
	glm::mat4 lightProjection, lightView;
	float near_plane = 1.0f, far_plane = 7.5f;
	float range = m_renderDistPix;
	lightProjection = glm::ortho(-range, range, -range, range, 0.0f, 6000.f);
	glm::vec3 level0(m_trueLocation.x, 0.f, m_trueLocation.z);
	lightView = glm::lookAt(-LIGHT_DIR * (float)m_renderDistPix + level0,
		level0,
		glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;

	m_textures.activateTextures();

	const auto &mode = m_modes[m_currentMode];
	float elapsed = m_clock.getElapsedTime().asSeconds();

	glEnable(GL_DEPTH_TEST);

	if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// ----------------------- Render shadow ------------------------ //
	if (!mode.withoutShadow)
	{
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		m_shadowFBO.bindFBO();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_shadowShader.use();
		m_shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		m_shadowShader.setMat4("model", m_mvp.model);
		m_chunks.renderTerrain(false);
	}
	// --------------------------------------------------------------- //

	//view = m_camera.getViewMatrix();
	m_terrainShader.use();
	m_terrainShader.setMat4("model", m_mvp.model);
	m_terrainShader.setMat4("view", m_mvp.view);
	m_terrainShader.setMat4("projection", m_mvp.projection);
	m_terrainShader.setVec3("viewPos", m_camera.Position);
	m_terrainShader.setFloat("viewDistance", 4 * Chunk::SIZE + (1.f - m_masterScale) * 20 * Chunk::SIZE);
	m_terrainShader.setFloat("renderMode", REFRACTION);
	m_terrainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	m_terrainShader.setBool("withoutShadow", mode.withoutShadow);
	m_terrainShader.setFloat("scale", m_masterScale);

	glViewport(0, 0, m_windowSize.x, m_windowSize.y);

	// ----------------- Render Refraction ----------------------- //
	if (!mode.withoutRefraction)
	{
		m_refractionFBO.bindFBO();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_chunks.renderTerrain();
	}


	m_reflectionFBO.bindFBO();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ------------------- Render reflected sky --------------- //
	glDepthMask(GL_FALSE);
	m_waterPlaneShader.use();
	m_waterPlaneShader.setMat4("model", m_mvp.model);
	m_waterPlaneShader.setMat4("view", m_mvp.view);
	m_waterPlaneShader.setMat4("projection", m_mvp.projection);
	m_waterPlaneShader.setVec3("viewPos", m_camera.Position);
	m_waterPlaneShader.setBool("withOcean", mode.withOcean);
	m_waterPlaneShader.setFloat("scale", m_masterScale);
	m_waterPlaneShader.setFloat("viewDistance", 4 * Chunk::SIZE + (1.f - m_masterScale) * 10 * Chunk::SIZE);
	m_waterPlaneShader.setFloat("time", elapsed / 100);

	m_chunks.renderWater({ &m_waterPlaneShader, m_mvp.model }, m_withOcean);

	glDepthMask(GL_TRUE);
	// --------------------------------------------------------- //

	// ---------------- Render Reflection ---------------------- //
	if (!mode.withoutReflection)
	{

		// ---------------- Render reflected terrain ---------------- //
		glCullFace(GL_FRONT);

		m_terrainShader.use();
		m_terrainShader.setFloat("renderMode", REFLECTION);
		m_chunks.renderTerrain();

		glCullFace(GL_BACK);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// ---------------- Render skybox ------------------ //
	glDepthMask(GL_FALSE);
	m_skyboxShader.use();
	auto staticView = glm::mat4(glm::mat3(m_camera.getViewMatrix())); // remove translation from the view matrix
	m_skyboxShader.setMat4("view", staticView);
	m_skyboxShader.setMat4("projection", m_mvp.projection);

	m_skybox.draw();
	glDepthMask(GL_TRUE);
	// ----------------------------------------------- //

	if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// -------------------- Render terrain -------------------- //
	m_terrainShader.use();
	m_terrainShader.setFloat("renderMode", FINAL_PASS);
	m_chunks.renderTerrain();
	// --------------------------------------------------------- //

	// ------------------ Render water -------------------- //
	m_textures.activateTextures();
	m_reflectionShader.use();
	m_reflectionShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	m_reflectionShader.setMat4("model", m_mvp.model);
	m_reflectionShader.setMat4("view", m_mvp.view);
	m_reflectionShader.setMat4("projection", m_mvp.projection);
	m_reflectionShader.setFloat("viewDistance", 4 * Chunk::SIZE + (1.f - m_masterScale) * 20 * Chunk::SIZE);
	m_reflectionShader.setVec3("viewPos", m_camera.Position);
	m_reflectionShader.setBool("withoutReflection", mode.withoutReflection);
	m_reflectionShader.setBool("withoutRefraction", mode.withoutRefraction);
	m_reflectionShader.setBool("withoutShadow", mode.withoutShadow);
	m_reflectionShader.setBool("withOcean", mode.withOcean);
	m_reflectionShader.setFloat("scale", m_masterScale);
	m_reflectionShader.setFloat("time", elapsed / 100);
	m_chunks.renderWater({ &m_reflectionShader, m_mvp.model }, m_withOcean);
	// ----------------------------------------------------- //


	// ---------------------- Render stats --------------------- //
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

const glm::mat4 Application::MVP::mvp() const
{
	return projection * view * model;
}
