#pragma once
#include <figure.h>

class Axis : public Figure {
public:
	Axis(float length = 5000, float width = 2.5f);

	void Render(int colorLoc, int modelLoc) override;

private:
	float length;
	float width;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
		InitializeAndCalculate(float length, float width);
};