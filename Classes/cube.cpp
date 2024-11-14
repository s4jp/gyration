#include "cube.h"
#include <glm/gtc/type_ptr.hpp>

static float yellow[4] = { 1.f, 1.f, 0.f, 0.5f };
static float white[4] = { 1.f, 1.f, 1.f, 0.5f };
static glm::vec3 baseDiagonal = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
static float GRAVITY = 9.81f;

Cube::Cube(float edgeLength, float deviation, float initAngleVelocity, float density, bool* gravity) : Figure(Calculate(), "Cube", glm::vec3(0.f))
{
	SetScale(glm::vec3(edgeLength / 2.f));
	SetDeviation(deviation);
	this->density = density;
	this->gravity = gravity;

	Q = glm::quat(1, 0, 1, 0);
	//Q = glm::normalize(Q);
	Q *= GetRotation();

	W = glm::vec3{ initAngleVelocity,initAngleVelocity,initAngleVelocity };
}

glm::vec3 Cube::GetN() const
{
	if (!&gravity)
		return glm::vec3(0.0f);

	glm::vec3 massCenter = Q * this->GetScale();

	glm::vec3 g = glm::vec3(-1.f);
	g = glm::normalize(g);
	g *= GRAVITY;

	glm::vec3 axis = glm::cross(massCenter, g);

	glm::quat n = glm::conjugate(Q) * glm::quat(0, axis.x, axis.y, axis.z) * Q;
	return glm::vec3(n.x, n.y, n.z);
}

glm::mat3 Cube::GetInertiaTensor() const
{
	glm::mat3 inertiaTensor =
	{
		2.f / 3,	-0.25f,		-0.25f,
		-0.25f,		2.f / 3,	-0.25f,
		-0.25f,		-0.25f,		2.f / 3
	};

	float edgeLength = this->GetScale().x * 2.f;
	float mass = density * edgeLength * edgeLength * edgeLength;
	return mass * edgeLength * edgeLength * inertiaTensor;
}

void Cube::CalculateNextStep(float dt)
{
	auto func = [&](const glm::vec3& W, const glm::quat& Q) {
		glm::vec3 dW_dt = GetWt(W);
		glm::quat dQ_dt = GetQt(Q, W);
		return std::make_pair(dW_dt, dQ_dt);
		};

	auto [k1_W, k1_Q] = func(W, Q);
	auto [k2_W, k2_Q] = func(W + 0.5f * dt * k1_W, Q + 0.5f * dt * k1_Q);
	auto [k3_W, k3_Q] = func(W + 0.5f * dt * k2_W, Q + 0.5f * dt * k2_Q);
	auto [k4_W, k4_Q] = func(W + dt * k3_W, Q + dt * k3_Q);

	W += (dt / 6.0f) * (k1_W + 2.0f * k2_W + 2.0f * k3_W + k4_W);
	Q += (dt / 6.0f) * (k1_Q + 2.0f * k2_Q + 2.0f * k3_Q + k4_Q);

	Q = glm::normalize(Q);
	CalculateModelMatrix();
}

void Cube::CalculateModelMatrix()
{
	Figure::CalculateModelMatrix();
	this->model *= glm::toMat4(Q);
}

void Cube::SetDeviation(float deviation)
{
	float angle = glm::radians(deviation);
	glm::vec3 newDiagonal = glm::normalize(glm::vec3(glm::cos(angle + M_PI_2), glm::sin(angle + M_PI_2), 0.f));
	SetRotation(glm::rotation(baseDiagonal, newDiagonal));
}

glm::vec3 Cube::GetWt(glm::vec3 W) const {
	glm::mat3 I = GetInertiaTensor();
	glm::vec3 N = GetN();
	glm::vec3 IW = I * W;
	glm::vec3 dW_dt = glm::inverse(I) * (N + glm::cross(IW, W));
	return dW_dt;
}

glm::quat Cube::GetQt(glm::quat Q, glm::vec3 W) const {
	glm::quat W_quat(0.0f, W.x, W.y, W.z);
	glm::quat dQ_dt = Q * 0.5f * W_quat;
	return dQ_dt;
}

// not simulation

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
		0, 1, 2, 3,			// Front wireframe
		4, 5, 6, 7,			// Back wireframe
		1, 5, 6, 2,			// Right wireframe
		4, 0, 3, 7,			// Left wireframe
		0, 6, 				// Diagonal
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
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * 52));

	vao.Unbind();
}