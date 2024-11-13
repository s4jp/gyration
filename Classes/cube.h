#pragma once
#include <figure.h>

class Cube : public Figure {
public:
	Cube(float edgeLength, float deviation = 0.f);

	void Render(int colorLoc, int modelLoc) override;
	void RenderDiagonal(int colorLoc, int modelLoc);

	void SetDeviation(float deviation);

private:
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
};