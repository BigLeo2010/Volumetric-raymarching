#include "GUI.h"
#include <string>

float GUI::isoMin = 0.2f;
float GUI::isoMax = 0.9f;

float GUI::clipX = 2.0f;
float GUI::clipY = 2.0f;
float GUI::clipZ = 2.0f;

bool GUI::smoothing = false;
bool GUI::use_dvr = false;

ImGradient GUI::gradient;
ImGradientMark* GUI::draggingMark = nullptr;
ImGradientMark* GUI::selectedMark = nullptr;
unsigned int GUI::colorID = 0;

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

    glGenTextures(1, &colorID);
    glBindTexture(GL_TEXTURE_1D, colorID);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    UpdateTransferFunctionTexture();
}

void GUI::UpdateTransferFunctionTexture() {
    float data[256 * 3];
    for (int i = 0; i < 256; ++i) {
        gradient.getColorAt(i / 255.0f, &data[i * 3]);
    }

    glBindTexture(GL_TEXTURE_1D, colorID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, 256, 0, GL_RGB, GL_FLOAT, data);
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
    shader.SetBool("STBNsmooth", smoothing);
    shader.SetBool("use_dvr", use_dvr);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_1D, colorID);
    shader.SetInt("colorRamp", 5);
}

void GUI::CreateGUI(double fps, Texture& noise3DTexture, bool& canRotate) {
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

        ImGui::Checkbox("STBN smooth", &smoothing);
        ImGui::Checkbox("Use DVR", &use_dvr);

        ImGui::Text("Virtual clipper:");
        ImGui::SliderFloat("X Axis", &clipX, -2.0f, 2.0f);
        ImGui::SliderFloat("Y Axis", &clipY, -2.0f, 2.0f);
        ImGui::SliderFloat("Z Axis", &clipZ, -2.0f, 2.0f);

        ImGui::Checkbox("Rotate scan", &canRotate);

        ImGui::Separator();
        ImGui::Text("Transfer Function:");
        if (ImGui::GradientEditor(&gradient, draggingMark, selectedMark)) {
            UpdateTransferFunctionTexture();
        }

        ImGui::End();
    }
}

void GUI::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GUI::~GUI() {
    if (colorID != 0) {
        glDeleteTextures(1, &colorID);
    }

    if (windowGUI) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
