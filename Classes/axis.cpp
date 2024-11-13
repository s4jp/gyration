#include "axis.h"
#include <glm/gtc/type_ptr.hpp>

static float red[4] = { 1.f, 0.f, 0.f, 1.f };
static float green[4] = { 0.f, 1.f, 0.f, 1.f };
static float blue[4] = { 0.f, 0.f, 1.f, 1.f };

Axis::Axis(float length, float width) : Figure(InitializeAndCalculate(length, width), "Axis", glm::vec3(0.f)) {}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Axis::InitializeAndCalculate(float length, float width)
{
	this->length = length;
	this->width = width;
	return Calculate();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Axis::Calculate() const
{
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	// x-axis
	vertices.push_back(-length);
	vertices.push_back(0.f);
	vertices.push_back(0.f);
	vertices.push_back(length);
	vertices.push_back(0.f);
	vertices.push_back(0.f);

	// y-axis
	vertices.push_back(0.f);
	vertices.push_back(-length);
	vertices.push_back(0.f);
	vertices.push_back(0.f);
	vertices.push_back(length);
	vertices.push_back(0.f);

	// z-axis
	vertices.push_back(0.f);
	vertices.push_back(0.f);
	vertices.push_back(-length);
	vertices.push_back(0.f);
	vertices.push_back(0.f);
	vertices.push_back(length);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(5);

	return std::make_tuple(vertices, indices);
}

void Axis::Render(int colorLoc, int modelLoc)
{
	vao.Bind();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glLineWidth(width);

	glUniform4fv(colorLoc, 1, red);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);

	glUniform4fv(colorLoc, 1, green);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(2 * sizeof(GLuint)));

	glUniform4fv(colorLoc, 1, blue);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(4 * sizeof(GLuint)));

	vao.Unbind();
}