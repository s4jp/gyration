#pragma once
#include <figure.h>

class Path : public Figure {
public:
	Path(int* length);

	void AddPoint(glm::vec3 point);
	void Clear();

	void Render(int colorLoc, int modelLoc) override;
private:
	std::vector<GLfloat> points;
	int* length;
};