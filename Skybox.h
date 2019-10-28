#pragma once
#include <vector>

class Skybox
{
public:
	void initialize(const std::vector<std::string> &faceFiles);
	void draw() const;

private:

	unsigned int m_skyboxVAO, m_skyboxVBO;
	unsigned int m_cubeTex;
};