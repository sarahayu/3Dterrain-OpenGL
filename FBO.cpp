#include "FBO.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include "Textures.h"

FBO::FBO(Textures & textures)
	: m_textures(textures)
{
}

void FBO::initialize(const glm::vec2 & dimensions)
{
	if (m_fbo) glDeleteFramebuffers(1, &m_fbo);

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	m_dimensions = dimensions;
}

const unsigned int FBO::attachColorBuffer(const WrapParam & params)
{
	unsigned int id;
	if (m_colorBufferOffset)
	{
		id = m_textures.reloadTexture(params, m_colorBufferOffset);
	}
	else
	{
		std::cout << "\nLoading...";
		auto color = m_textures.loadTexture(params);
		m_colorBufferOffset = color.offset;
		id = color.id;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_dimensions.x, m_dimensions.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
	return m_colorBufferOffset;
}

const unsigned int FBO::attachDepthBuffer(const WrapParam & params)
{
	unsigned int id;
	if (m_depthBufferOffset)
	{
		id = m_textures.reloadTexture(params, m_depthBufferOffset);
	}
	else
	{
		std::cout << "\nLoading...";
		auto depth = m_textures.loadTexture(params);
		m_depthBufferOffset = depth.offset;
		id = depth.id;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_dimensions.x, m_dimensions.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id, 0);
	return m_depthBufferOffset;
}

void FBO::attachRenderBuffer()
{
	if (m_rbo) glDeleteRenderbuffers(1, &m_rbo);
	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_dimensions.x, m_dimensions.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo); // now actually attach it

}

void FBO::check() const
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}

void FBO::bindFBO() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}