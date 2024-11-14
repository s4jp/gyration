#include "cube.h"
#include <glm/gtc/type_ptr.hpp>

static float yellow[4] = { 1.f, 1.f, 0.f, 0.5f };
static float white[4] = { 1.f, 1.f, 1.f, 0.5f };
static float purple[4] = { 1.f, 0.f, 1.f, 0.7f };

static glm::vec3 baseDiagonal = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
static float GRAVITY = 9.81f;

Cube::Cube(float edgeLength, float deviation) : Figure(Calculate(), "Cube", glm::vec3(0.f))
{
	SetScale(glm::vec3(edgeLength / 2.f));
	SetDeviation(deviation);

	Q = glm::quat();
}

void Cube::SetDeviation(float deviation)
{
	float angle = glm::radians(deviation);
	glm::vec3 newDiagonal = glm::normalize(glm::vec3(glm::cos(angle + M_PI_2), glm::sin(angle + M_PI_2), 0.f));
	SetRotation(glm::rotation(baseDiagonal, newDiagonal));
}

void Cube::SetQ(glm::quat Q)
{
	this->Q = Q;
	CalculateModelMatrix();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Cube::Calculate() const
{
	std::vector<GLfloat> vertices =
	{
		0,		0,		0,		// 0
		1,		0,		0,		// 1
		1,		1,		0,		// 2
		0,		1,		0,		// 3
		0,		0,		1,		// 4
		1,		0,		1,		// 5
		1,		1,		1,		// 6
		0,		1,		1,		// 7

		0.5f,	0.5f,	0.5f,	// 8
		0.5f,	0.5f,	0.5f,	// 9
		0.5f,	0.5f,	0.5f,	// 10
		0.5f,	0.5f,	0.5f,	// 11
		0.5f,	0.5f,	0.5f,	// 12
	};
	std::vector<GLuint> indices =
	{
		0, 2, 1, 2, 0, 3,	// Front face
		1, 6, 5, 2, 6, 1,	// Right face
		6, 7, 5, 7, 4, 5,	// Back face
		4, 3, 0, 4, 7, 3,	// Left face
		3, 6, 2, 3, 7, 6,	// Top face
		4, 1, 5, 4, 0, 1,	// Bottom face
		0, 1, 2, 3,			// Front wireframe
		4, 5, 6, 7,			// Back wireframe
		1, 5, 6, 2,			// Right wireframe
		4, 0, 3, 7,			// Left wireframe
		0, 6, 				// Diagonal
		8, 9, 10, 11, 12,	// Gravity vector
	};

	return std::make_tuple(vertices, indices);
}

void Cube::Render(int colorLoc, int modelLoc)
{
	vao.Bind();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glUniform4fv(colorLoc, 1, white);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * 36));
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * 40));
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * 44));
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * 48));

	vao.Unbind();
}

void Cube::RenderDiagonal(int colorLoc, int modelLoc)
{
	vao.Bind();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(colorLoc, 1, yellow);
	glLineWidth(1.f);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * 52));

	vao.Unbind();
}

void Cube::RenderGravity(int colorLoc, int modelLoc)
{
	vao.Bind();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(colorLoc, 1, purple);
	glLineWidth(2.f);
	glDrawElements(GL_LINE_STRIP, 5, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * 54));

	vao.Unbind();
}

glm::vec3 Cube::GetSamplePoint() const
{
	return glm::vec3(model * glm::vec4(1.f));
}

void Cube::CalculateModelMatrix()
{
	Figure::CalculateModelMatrix();
	this->model *= glm::toMat4(Q);
}