#include "path.h"
#include <glm/gtc/type_ptr.hpp>

static float white[4] = { 1.f, 1.f, 1.f, 0.5f };

Path::Path(int* length) : Figure({}, "Path", glm::vec3(0.f))
{
	this->length = length;
	points = {};
}

void Path::AddPoint(glm::vec3 point)
{
	int surplus = points.size() - (*length + 1) * 3;
	if (surplus > 0)
	{
		points.erase(points.begin(), points.begin() + surplus);
	}

	points.push_back(point.x);
	points.push_back(point.y);
	points.push_back(point.z);

	RefreshBuffers({ points, {} });
}

void Path::Render(int colorLoc, int modelLoc)
{
	vao.Bind();

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(colorLoc, 1, white);
	glDrawArrays(GL_LINE_STRIP, 0, points.size() / 3);

	vao.Unbind();
}