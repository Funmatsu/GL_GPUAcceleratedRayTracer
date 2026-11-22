#include "Mesh.h"

Mesh::Mesh() {
	idxcnt = 0;
	vao = 0;
	vbo = 0;
	ibo = 0;
}
void Mesh::sendMesh(vector<float>& vertices, vector<unsigned int>& indices) {
	idxcnt = indices.size();

	if (vao == 0)
		glGenVertexArrays(1, &vao);
	if (vbo == 0) 
		glGenBuffers(1, &vbo);
	if (ibo == 0)
		glGenBuffers(1, &ibo);

	glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, 0, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxcnt * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);
}
void Mesh::drawMesh() {
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, idxcnt, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}