#pragma once
#include <array>

class ScreenOverlay
{
public:
	void create();
	void render();

private:
	unsigned int m_vao;
	unsigned int m_vbo;
};