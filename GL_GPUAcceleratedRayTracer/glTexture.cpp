#include "glTexture.h"
#include <iostream>

glTexture::glTexture(int width, int height) {
	w = width, h = height; txbo = 0;

	glGenTextures(1, &txbo);
	glBindTexture(GL_TEXTURE_2D, txbo);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);
}
void glTexture::use() {
	glBindImageTexture(0, txbo, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
}

void glTexture::bind() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, txbo);
}