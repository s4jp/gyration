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
  glm::vec3 scale;
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

  void virtual CalculateModelMatrix() {
      glm::mat4 translateM =
          CAD::translate(glm::mat4(1.0f), GetPosition());
      glm::mat4 scaleM = CAD::scaling(glm::mat4(1.0f), GetScale());

      model = translateM * scaleM;
  }

  Figure(std::tuple<std::vector<GLfloat>, std::vector<GLuint>> data, std::string type, glm::vec3 center) {
      scale = glm::vec3(1.0f);
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
  glm::vec3 GetScale() const { return scale; }
  glm::vec3 GetPosition() const { return center; }

  void SetScale(glm::vec3 nScale) {
      scale = nScale;
      CalculateModelMatrix();
  }
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