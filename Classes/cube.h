#pragma once
#include <figure.h>

class Cube : public Figure {
public:
	Cube(float edgeLength, float deviation, float initAngleVelocity, float denisty, bool* gravity);

	void Render(int colorLoc, int modelLoc) override;
	void RenderDiagonal(int colorLoc, int modelLoc);
	void RenderGravity(int colorLoc, int modelLoc);

	void SetDeviation(float deviation);
	void CalculateNextStep(float dt);
	glm::vec3 GetSamplePoint() const;

private:
	glm::quat Q;
	glm::vec3 W;
	float density;
	bool* gravity;

	glm::mat3 GetInertiaTensor() const;
	glm::vec3 GetN() const;
	glm::vec3 GetWt(glm::vec3 W) const;
	glm::quat GetQt(glm::quat Q, glm::vec3 W) const;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
	void virtual CalculateModelMatrix() override;
};