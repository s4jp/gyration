#pragma once
#define _USE_MATH_DEFINES
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "helpers.h"

#include <glm/glm.hpp>
#include <tuple>
#include <string>

class Figure
{
private:
  glm::vec3 center;

protected:
  VAO vao;
  VBO vbo;
  EBO ebo;

  glm::mat4 model;
  size_t indices_count;

public:
  std::string name;

  virtual void Render(int colorLoc, int modelLoc) = 0;

  void Delete() {
    vao.Delete();
    vbo.Delete();
    ebo.Delete();
  }

  void CalculateModelMatrix() {
    model = CAD::translate(glm::mat4(1.0f), GetPosition());
  }

  Figure(std::tuple<std::vector<GLfloat>, std::vector<GLuint>> data, std::string type, glm::vec3 center) {
    this->center = center;
    indices_count = std::get<1>(data).size();
    model = glm::mat4(1.0f);

    vao.Bind();
    vbo = VBO(std::get<0>(data).data(), std::get<0>(data).size() * sizeof(GLfloat));
    ebo = EBO(std::get<1>(data).data(), std::get<1>(data).size() * sizeof(GLint));

    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 0, (void *)0);
    vao.Unbind();
    vbo.Unbind();
    ebo.Unbind();

    name = type;

    CalculateModelMatrix();
  }

  glm::vec3 GetPosition() const { return center; }

  void SetPosition(glm::vec3 nPosition) {
    center = nPosition;
    CalculateModelMatrix();
  }

  void RefreshBuffers(std::tuple<std::vector<GLfloat>, std::vector<GLuint>> data) {
      indices_count = std::get<1>(data).size();
	  vao.Bind();
      vbo.ReplaceBufferData(std::get<0>(data).data(), std::get<0>(data).size() * sizeof(GLfloat));
      ebo.ReplaceBufferData(std::get<1>(data).data(), std::get<1>(data).size() * sizeof(GLint));
	  vao.Unbind();
  }
};