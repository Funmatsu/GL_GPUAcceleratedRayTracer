#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

class glShader
{
public:
	glShader();
	void createFromFiles(string vertexCode, string fragmentpath);
	void createFromFiles(string computeCode);

	string parsefile(string path);

	unsigned int compile(GLenum type, const char* code);
	void compileNlinkShader(const char* vertexshader, const char* fragmentshader);
	void compileNlinkShader(const char* computeshader);

	unsigned int getid() { return shid; }
	void use();
private:
	unsigned int shid;
};

