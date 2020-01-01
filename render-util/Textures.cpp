#include "Textures.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

const unsigned int Textures::generateTexture(const WrapParam & params)
{
	return params.filterMag == 0 ? generateTexture(params.wrap, params.filterMin, params.filterMin)
		: generateTexture(params.wrap, params.filterMin, params.filterMag);
}

const unsigned int Textures::generateTexture(const unsigned int & wrap, const unsigned int & filterMin, const unsigned int & filterMag)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMag);
	
	return texture;
}

const Textures::TextureData Textures::loadTexture(const WrapParam &params)
{
	return params.filterMag == 0 ? generate(params.wrap, params.filterMin, params.filterMin)
		: generate(params.wrap, params.filterMin, params.filterMag);
}

const unsigned int Textures::reloadTexture(const WrapParam & params, const unsigned int & offset)
{
	assert(offset < m_textures.size());
	return m_textures[offset] = generateTexture(params);
}

const Textures::TextureData Textures::generate(const unsigned int &wrap, const unsigned int &filterMin, const unsigned int &filterMag)
{
	unsigned int texture = generateTexture(wrap, filterMin, filterMag);

	m_textures.push_back(texture);

	return{ m_texOffset++, texture };
}

const Textures::TextureData Textures::loadTex2D(const std::string & filename)
{
	auto texData = loadTexture({ GL_REPEAT, GL_NEAREST, GL_LINEAR });

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);	

	return texData;
}

void Textures::activateTextures() const
{
	for (int i = 0; i < m_textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
	}
}
