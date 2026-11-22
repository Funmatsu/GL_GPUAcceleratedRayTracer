#pragma once
#include <stb_image.h>
#define GLEW_STATIC
#include <GL/glew.h>

class glTexture
{
public:
	glTexture(int w, int h);
	void use();
	void bind();
	unsigned int getbo() { return txbo; }
private:
	unsigned int txbo,
		         w, h;
};