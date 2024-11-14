#include "plane.h"
#include <glm/gtc/type_ptr.hpp>

static float gray[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

Plane::Plane(float size) : Figure(InitializeAndCalculate(size), "Plane", glm::vec3(0.f)) {}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Plane::InitializeAndCalculate(float size)
{
	this->size = size;
	return Calculate();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Plane::Calculate() const
{
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	// vertices
	vertices.push_back(-size);
	vertices.push_back(0.f);
	vertices.push_back(-size);

	vertices.push_back(size);
	vertices.push_back(0.f);
	vertices.push_back(-size);

	vertices.push_back(size);
	vertices.push_back(0.f);
	vertices.push_back(size);

	vertices.push_back(-size);
	vertices.push_back(0.f);
	vertices.push_back(size);

	// indices
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(0);

	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	return std::make_tuple(vertices, indices);
}

void Plane::Render(int colorLoc, int modelLoc)
{
	vao.Bind();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(colorLoc, 1, gray);
	glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, 0);

	vao.Unbind();
}