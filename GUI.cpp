#include "GUI.h"
#include <string>

float GUI::isoMin = 0.2f;
float GUI::isoMax = 0.9f;

float GUI::clipX = 2.0f;
float GUI::clipY = 2.0f;
float GUI::clipZ = 2.0f;

GUI::GUI() {}

void GUI::InitGUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    io = &ImGui::GetIO();

    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    windowGUI = window;
}

void GUI::UIInputs() {
    if (!windowGUI || !io) return;

    bool isRightMouseDown = (glfwGetMouseButton(windowGUI, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

    if (!isRightMouseDown)
    {
        glfwSetInputMode(windowGUI, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        io->ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    }
    else
    {
        glfwSetInputMode(windowGUI, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        io->ConfigFlags |= ImGuiConfigFlags_NoMouse;
    }
}

void GUI::UniformValues(Shader& shader) {
    shader.SetFloat("isoMax", isoMax);
    shader.SetFloat("isoMin", isoMin);
	shader.SetFloat("clipX", clipX);
    shader.SetFloat("clipY", clipY);
    shader.SetFloat("clipZ", clipZ);
}

void GUI::CreateGUI(double fps, Texture& noise3DTexture, glm::vec3& colorA, glm::vec3& colorB, bool& canRotate) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Scene");

        std::string fpsTitle = "FPS: " + std::to_string(int(fps));
        ImGui::Text("%s", fpsTitle.c_str());

        ImGui::Text("Main algorithm:");
        ImGui::SliderFloat("IsoMin", &isoMin, 0.0f, 1.0f);
        ImGui::SliderFloat("IsoMax", &isoMax, 0.0f, 1.0f);

        ImGui::Text("Virtual clipper:");
        ImGui::SliderFloat("X Axis", &clipX, -2.0f, 2.0f);
        ImGui::SliderFloat("Y Axis", &clipY, -2.0f, 2.0f);
        ImGui::SliderFloat("Z Axis", &clipZ, -2.0f, 2.0f);

        ImGui::Checkbox("Rotate scan", &canRotate);

        ImGui::Text("Color:");
        ImGui::ColorEdit3("A", &colorA.r);
        ImGui::ColorEdit3("B", &colorB.r);

        ImGui::End();
    }
}

void GUI::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GUI::~GUI() {
    if (windowGUI) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
