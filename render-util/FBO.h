#pragma once
#include <glm/glm.hpp>
#include "Textures.h"

class FBO
{
public:
	FBO(Textures &textures);
	void initialize(const glm::vec2 &dimensions);
	const unsigned int attachColorBuffer( const WrapParam &params);		// return texture offset
	const unsigned int attachDepthBuffer(const WrapParam &params);		// return texture offset
	void attachRenderBuffer();
	void check() const;
	void bindFBO() const;

private:

	Textures &m_textures;
	glm::vec2 m_dimensions;
	unsigned int m_fbo;
	unsigned int m_rbo;

	// disregard if not used
	unsigned int m_colorBufferOffset = 0;
	unsigned cid;
	unsigned int m_depthBufferOffset = 0;
	unsigned int did;
};