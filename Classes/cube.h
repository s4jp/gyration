#pragma once
#include <figure.h>

class Cube : public Figure {
public:
	Cube(float edgeLength);

	void Render(int colorLoc, int modelLoc) override;
	void RenderDiagonal(int colorLoc, int modelLoc);

private:
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
};