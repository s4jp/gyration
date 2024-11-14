#pragma once
#include <figure.h>

class Plane : public Figure {
public:
	Plane(float size);

	void Render(int colorLoc, int modelLoc) override;

private:
	float size;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
		InitializeAndCalculate(float size);
};