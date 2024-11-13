#define _USE_MATH_DEFINES
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Camera.h"

#include "ControlledInputFloat.h"
#include "ControlledInputInt.h"

const float near = 0.1f;
const float far = 100.0f;

Camera *camera;

glm::mat4 view;
glm::mat4 proj;

bool running = false;
ControlledInputFloat edgeLength("Edge Length", 1.0f, 0.1f, 0.1f);
ControlledInputFloat density("Density", 1.0f, 0.1f, 0.1f);
ControlledInputFloat deviation("Deviation", 15.0f, 0.1f, 0.1f);
ControlledInputFloat angularVelocity("Ang. Vel.", 15.0f, 0.1f, 0.1f);
ControlledInputFloat integrationStep("Int. Step", 0.001f, 0.0001f, 0.0001f);
ControlledInputInt pathLength("Path Length", 5000, 10, 1);
bool showCube = true;
bool showDiagonal = true;
bool showPath = true;
bool showGravity = true;
bool gravity = true;
static float color[3] = { 0.f, 0.f, 1.f };

void window_size_callback(GLFWwindow *window, int width, int height);

int modelLoc, viewLoc, projLoc, colorLoc;

int main() { 
    // initial values
    int width = 1500;
    int height = 800;
    glm::vec3 cameraPosition = glm::vec3(3.0f, 3.0f, 3.0f);
    float fov = M_PI / 4.0f;
    int guiWidth = 300;

    #pragma region gl_boilerplate
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, "Gyration", NULL, NULL);
    if (window == NULL) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, width - guiWidth, height);
    glEnable(GL_DEPTH_TEST);

    GLFWimage icon;
    icon.pixels = stbi_load("icon.png", &icon.width, &icon.height, 0, 4);
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);
    #pragma endregion

    // shaders and uniforms
    Shader shaderProgram("Shaders\\default.vert", "Shaders\\default.frag");
    modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
    viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
    projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
    colorLoc = glGetUniformLocation(shaderProgram.ID, "color");

    // callbacks
    glfwSetWindowSizeCallback(window, window_size_callback);

    camera = new Camera(width, height, cameraPosition, fov, near, far, guiWidth);
    camera->PrepareMatrices(view, proj);

    #pragma region imgui_boilerplate
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    #pragma endregion

    while (!glfwWindowShouldClose(window))
    {
#pragma region init
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(camera->guiWidth, camera->GetHeight()));
        ImGui::SetNextWindowPos(ImVec2(camera->GetWidth(), 0));
#pragma endregion

        camera->HandleInputs(window);
        camera->PrepareMatrices(view, proj);

        // render non-grayscaleable objects
        shaderProgram.Activate();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
        // render


        // imgui rendering
        ImGui::Begin("Menu", 0,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (running) {
            if (ImGui::Button("Stop simulation", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                running = false;
                // TODO: stop the simulation
            }
        }
        else {
            if (ImGui::Button("Start simulation", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                running = true;
                // TODO: start the simulation
            }
        }

        ImGui::SeparatorText("Initial conditions");
        edgeLength.render();
        // TODO: recalculate inertia tensor upon change
		density.render();
		// TODO: recalculate inertia tensor upon change
		deviation.render();
		angularVelocity.render();
		integrationStep.render();
        if (ImGui::Button("Apply changes", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			// TODO: apply changes
        }

		ImGui::SeparatorText("Visualization");
		pathLength.render();
        ImGui::Checkbox("Show cube", &showCube);
        ImGui::SameLine(std::max(ImGui::GetWindowWidth() / 2.f, ImGui::CalcTextSize("Show cube").x));
		ImGui::Checkbox("Show diagonal", &showDiagonal);
		ImGui::Checkbox("Show path", &showPath);
        ImGui::SameLine(std::max(ImGui::GetWindowWidth() / 2.f, ImGui::CalcTextSize("Show path").x));
		ImGui::Checkbox("Show gravity", &showGravity);

        ImGui::SeparatorText("Other");
		ImGui::Checkbox("Gravity", &gravity);
		ImGui::ColorEdit3("Cube color", color);

        ImGui::End();
        #pragma region rest
        ImGui::Render();
        //std::cout << ImGui::GetIO().Framerate << std::endl;
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
        #pragma endregion
    }
    #pragma region exit
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    shaderProgram.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
    #pragma endregion
}

// callbacks
void window_size_callback(GLFWwindow *window, int width, int height) {
  camera->SetWidth(width);
  camera->SetHeight(height);
  camera->PrepareMatrices(view, proj);
  glViewport(0, 0, width - camera->guiWidth, height);
}