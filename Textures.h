#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <SFML\Graphics.hpp>

struct WrapParam
{
	unsigned int wrap;
	unsigned int filterMin;
	unsigned int filterMag;
};

class Textures
{
public:
	struct TextureData
	{
		unsigned int offset;
		unsigned int id;
	};

	static const unsigned int generateTexture(const WrapParam &params);
	static const unsigned int generateTexture(const unsigned int &wrap, const unsigned int &filterMin, const unsigned int &filterMag);	

	const TextureData loadTexture(const WrapParam &params);
	const unsigned int reloadTexture(const WrapParam &params, const unsigned int &offset);
	const TextureData loadTex2D(const std::string &filename);

	void activateTextures() const;

private:

	const TextureData generate(const unsigned int &wrap, const unsigned int &filterMin, const unsigned int &filterMag);

	unsigned int m_texOffset;
	// [offset] = id
	std::vector<unsigned int> m_textures;
};