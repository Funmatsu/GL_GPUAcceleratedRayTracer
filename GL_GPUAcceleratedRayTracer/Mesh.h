#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <vector>

using namespace std;

class Mesh
{
public:
	Mesh();
	void sendMesh(vector<float>& vertices, vector<unsigned int>& indices);
	void drawMesh();
private:
	unsigned int vao, vbo, ibo, idxcnt;
};

