#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <atomic>
#include <mutex>

static glm::vec3 baseDiagonal = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
static glm::vec3 diviationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
static float GRAVITY = 9.81f;

glm::quat GetDeviation(float deviation)
{
	float angle = glm::radians(deviation);
	glm::vec3 newDiagonal = glm::normalize(glm::vec3(glm::cos(angle + M_PI_2), glm::sin(angle + M_PI_2), 0.f));
	return glm::rotation(baseDiagonal, newDiagonal);
}

struct SymParams {
	float size;
	float density;
	float dt;

	SymParams(float size, float density, float dt) : 
		size(size), density(density), dt(dt) {}
};

struct SymMemory {
	SymParams params;
	glm::quat Q;
	glm::vec3 W;
	bool gravity;
	float time;
	std::mutex mutex;
	std::atomic<bool> stopThread;
	std::atomic<float> sleep_debt;

	SymMemory(float size, float density, float deviation, float angularVelocity, float dt, bool gravity) : 
		params(size, density, dt)
	{
		this->gravity = gravity;

		Reset(deviation, angularVelocity);
	}

	~SymMemory()
	{
		stopThread = true;
	}

	void Reset(float deviation, float angularVelocity)
	{
		stopThread = false;
		sleep_debt = 0.0f;
		time = 0.0f;

		Q = glm::angleAxis(glm::radians(deviation), diviationAxis);
		W = glm::vec3{ angularVelocity, angularVelocity, angularVelocity };
	}
};

glm::vec3 GetN(bool gravity, glm::quat Q, float size)
{
	if (!gravity)
		return glm::vec3(0.0f); 

	glm::vec3 massCenter = Q * glm::vec3(size / 2.f);

	glm::vec3 g = glm::vec3(-1.f);
	g = glm::normalize(g);
	g *= GRAVITY;

	glm::vec3 axis = glm::cross(massCenter, g);

	glm::quat n = glm::conjugate(Q) * glm::quat(0, axis.x, axis.y, axis.z) * Q;
	return glm::vec3(n.x, n.y, n.z);
}

glm::mat3 GetInertiaTensor(float size, float density)
{
	glm::mat3 inertiaTensor =
	{
		2.f / 3,	-0.25f,		-0.25f,
		-0.25f,		2.f / 3,	-0.25f,
		-0.25f,		-0.25f,		2.f / 3
	};

	float edgeLength = size;
	float mass = density * edgeLength * edgeLength * edgeLength;
	return mass * edgeLength * edgeLength * inertiaTensor;
}

glm::vec3 GetWt(glm::vec3 W, float size, float density, bool gravity, glm::quat Q){
	glm::mat3 I = GetInertiaTensor(size, density);
	glm::vec3 N = GetN(gravity, Q, size);
	glm::vec3 IW = I * W;
	glm::vec3 dW_dt = glm::inverse(I) * (N + glm::cross(IW, W));
	return dW_dt;
}

glm::quat GetQt(glm::quat Q, glm::vec3 W){
	glm::quat W_quat(0.0f, W.x, W.y, W.z);
	glm::quat dQ_dt = Q * 0.5f * W_quat;
	return dQ_dt;
}

void calculationThread(SymMemory* memory)
{
	std::chrono::high_resolution_clock::time_point calc_start, calc_end, wait_start;

	while (!memory->stopThread) {
		calc_start = std::chrono::high_resolution_clock::now();

		memory->mutex.lock();

			float dt = memory->params.dt / 1000.f;
			memory->time += dt;

			auto func = [&](const glm::vec3& W, const glm::quat& Q) {
				glm::vec3 dW_dt = GetWt(W, memory->params.size, memory->params.density, memory->gravity, memory->Q);
				glm::quat dQ_dt = GetQt(Q, W);
				return std::make_pair(dW_dt, dQ_dt);
				};

			//auto [k1_W, k1_Q] = func(memory->W, memory->Q);
			//auto [k2_W, k2_Q] = func(memory->W + 0.5f * dt * k1_W, memory->Q + 0.5f * dt * k1_Q);
			//auto [k3_W, k3_Q] = func(memory->W + 0.5f * dt * k2_W, memory->Q + 0.5f * dt * k2_Q);
			//auto [k4_W, k4_Q] = func(memory->W + dt * k3_W, memory->Q + dt * k3_Q);

			auto [k1_W, k1_Q] = func(memory->W, memory->Q);
			glm::vec3 W_k2 = memory->W + 0.5f * dt * k1_W;
			auto [k2_W, k2_Q] = func(W_k2, memory->Q + 0.5f * dt * k1_Q);
			glm::vec3 W_k3 = memory->W + 0.5f * dt * k2_W;
			auto [k3_W, k3_Q] = func(W_k3, memory->Q + 0.5f * dt * k2_Q);
			glm::vec3 W_k4 = memory->W + dt * k3_W;
			auto [k4_W, k4_Q] = func(W_k4, memory->Q + dt * k3_Q);

			memory->W += (dt / 6.0f) * (k1_W + 2.0f * k2_W + 2.0f * k3_W + k4_W);
			memory->Q += (dt / 6.0f) * (k1_Q + 2.0f * k2_Q + 2.0f * k3_Q + k4_Q);

			memory->Q = glm::normalize(memory->Q);

		memory->mutex.unlock();

		calc_end = std::chrono::high_resolution_clock::now();

		float time2sleep = memory->params.dt * 1000000.f - std::chrono::duration_cast<std::chrono::nanoseconds>(calc_end - calc_start).count() - memory->sleep_debt;

		wait_start = std::chrono::high_resolution_clock::now();

		if (time2sleep > 0) {
			while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - wait_start).count() < time2sleep) {
				// busy waiting because std::this_thread::sleep_for sucks ass
			}
		}

		memory->sleep_debt = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - wait_start).count() - time2sleep;
	}
}