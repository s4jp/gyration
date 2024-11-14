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
#include "Axis.h"
#include "Cube.h"
#include "Plane.h"
#include "Path.h"
#include "symulator.h"

const float near = 0.1f;
const float far = 10000.0f;

Camera *camera;
Axis* axis;
Cube* cube;
Plane* plane;
Path* path;

glm::mat4 view;
glm::mat4 proj;

bool running = false;
ControlledInputFloat edgeLength("Edge Length", 2.0f, 0.1f, 0.1f);
ControlledInputFloat density("Density", 1.0f, 0.1f, 0.1f);
ControlledInputFloat deviation("Deviation", 15.0f, 0.1f);
ControlledInputFloat angularVelocity("Ang. Vel.", 15.0f, 0.1f, 0.1f);
ControlledInputFloat integrationStep("Step (ms)", 1.f, 0.1f, 0.1f);
ControlledInputInt pathLength("Path Length", 1000, 10, 1);
bool showCube = true;
bool showDiagonal = true;
bool showPath = true;
bool showGravity = true;
bool gravity = true;
static float color[4] = { 0.f, 0.f, 1.f, 0.4f };

std::chrono::high_resolution_clock::time_point realTime;
float simulationTime = 0;

void window_size_callback(GLFWwindow *window, int width, int height);

int modelLoc, viewLoc, projLoc, colorLoc, gravityLoc;

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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
	gravityLoc = glGetUniformLocation(shaderProgram.ID, "gravity");

    // callbacks
    glfwSetWindowSizeCallback(window, window_size_callback);

    camera = new Camera(width, height, cameraPosition, fov, near, far, guiWidth);
    camera->PrepareMatrices(view, proj);
	axis = new Axis();
	cube = new Cube(edgeLength.GetValue());
	plane = new Plane(edgeLength.GetValue() / 2.f);
	path = new Path(pathLength.GetPointer());

	SymMemory symMemory(edgeLength.GetValue(), density.GetValue(), deviation.GetValue(), angularVelocity.GetValue(), integrationStep.GetValue(), gravity);
    std::thread calcThread;

    #pragma region imgui_boilerplate
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    #pragma endregion

	calcThread = std::thread(calculationThread, &symMemory);
    realTime = std::chrono::high_resolution_clock::now();
    glm::quat Q;

    while (!glfwWindowShouldClose(window))
    {
        #pragma region init
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(camera->guiWidth, camera->GetHeight()));
        ImGui::SetNextWindowPos(ImVec2(camera->GetWidth(), 0));
        #pragma endregion

        camera->HandleInputs(window);
        camera->PrepareMatrices(view, proj);

		symMemory.mutex.lock();
		    Q = symMemory.Q;
            simulationTime = symMemory.time;
		symMemory.mutex.unlock();
		cube->SetQ(Q);
        path->AddPoint(cube->GetSamplePoint());

        // render non-grayscaleable objects
        shaderProgram.Activate();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
		glUniform1i(gravityLoc, false);

        // render
		axis->Render(colorLoc, modelLoc);
		if (showCube) {
            glUniform4fv(colorLoc, 1, color);
			cube->Render(colorLoc, modelLoc);
		}
		if (showDiagonal) {
			cube->RenderDiagonal(colorLoc, modelLoc);
		}
		if (showGravity) {
            glUniform1i(gravityLoc, true);
			cube->RenderGravity(colorLoc, modelLoc);
            glUniform1i(gravityLoc, false);
			plane->Render(colorLoc, modelLoc);
		}
		if (showPath) {
			path->Render(colorLoc, modelLoc);
		}

        // imgui rendering
        ImGui::Begin("Menu", 0,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar);

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
        edgeLength.Render();
		density.Render();
		deviation.Render();
		angularVelocity.Render();
		integrationStep.Render();
        ImGui::Checkbox("Gravity", &gravity);
        if (ImGui::Button("Apply changes", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
        {
            cube->SetScale(glm::vec3(edgeLength.GetValue() / 2.f));
			
			symMemory.mutex.lock();
			    symMemory.params.size = edgeLength.GetValue();
			    symMemory.params.density = density.GetValue();
			    symMemory.params.deviation = deviation.GetValue();
			    symMemory.params.angularVelocity = angularVelocity.GetValue();
			    symMemory.params.dt = integrationStep.GetValue();
			    symMemory.params.gravity = gravity;
			symMemory.mutex.unlock();
        }

		ImGui::SeparatorText("Visualization");
		pathLength.Render();
        ImGui::Checkbox("Show cube", &showCube);
        ImGui::SameLine(std::max(ImGui::GetWindowWidth() / 2.f, ImGui::CalcTextSize("Show cube").x));
		ImGui::Checkbox("Show diagonal", &showDiagonal);
		ImGui::Checkbox("Show path", &showPath);
        ImGui::SameLine(std::max(ImGui::GetWindowWidth() / 2.f, ImGui::CalcTextSize("Show path").x));
		ImGui::Checkbox("Show gravity", &showGravity);

        ImGui::SeparatorText("Other");
		ImGui::ColorEdit3("Cube color", color);

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 3 * ImGui::GetTextLineHeightWithSpacing());
        ImGui::Text(("t_sim = " + std::to_string(simulationTime) + "s").c_str());
        float real_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - realTime).count() / 1000.f;
        ImGui::Text(("t_real = " + std::to_string(real_time) + "s").c_str());
        ImGui::Text(("t_diff = " + std::to_string(-(real_time - simulationTime) / simulationTime * 100) + "%%").c_str());

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
	axis->Delete();
	cube->Delete();
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