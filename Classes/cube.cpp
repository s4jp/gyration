#include "cube.h"
#include <glm/gtc/type_ptr.hpp>

static float yellow[4] = { 1.f, 1.f, 0.f, 0.5f };

Cube::Cube(float edgeLength) : Figure(Calculate(), "Cube", glm::vec3(0.f)) 
{
	SetScale(glm::vec3(edgeLength / 2.f));
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Cube::Calculate() const
{
	std::vector<GLfloat> vertices = 
	{ 
		0,	0,	0,			// 0
		1,	0,	0,			// 1
		1,	1,	0,			// 2
		0,	1,	0,			// 3
		0,	0,	1,			// 4
		1,	0,	1,			// 5
		1,	1,	1,			// 6
		0,	1,	1			// 7
	};
	std::vector<GLuint> indices =
	{
		0, 2, 1, 2, 0, 3,	// Front face
		1, 6, 5, 2, 6, 1,	// Right face
		6, 7, 5, 7, 4, 5,	// Back face
		4, 3, 0, 4, 7, 3,	// Left face
		3, 6, 2, 3, 7, 6,	// Top face
		4, 1, 5, 4, 0, 1,	// Bottom face
		0, 6 				// Diagonal
	};

	return std::make_tuple(vertices, indices);
}

void Cube::Render(int colorLoc, int modelLoc)
{
	vao.Bind();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glDrawElements(GL_TRIANGLES, indices_count - 2, GL_UNSIGNED_INT, 0);

	vao.Unbind();
}

void Cube::RenderDiagonal(int colorLoc, int modelLoc)
{
	vao.Bind();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(colorLoc, 1, yellow);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * (indices_count - 2)));

	vao.Unbind();
}
