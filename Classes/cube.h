#pragma once
#include <figure.h>

class Cube : public Figure {
public:
	Cube(float edgeLength, float deviation);

	void SetDeviation(float deviation);
	glm::vec3 GetSamplePoint() const;
	void SetQ(glm::quat Q);

	void Render(int colorLoc, int modelLoc) override;
	void RenderDiagonal(int colorLoc, int modelLoc);
	void RenderGravity(int colorLoc, int modelLoc);

private:
	glm::quat Q;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
	void virtual CalculateModelMatrix() override;
};