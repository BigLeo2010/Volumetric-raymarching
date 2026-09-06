#include "GUI.h"
#include <string>

float GUI::density_value = 0.5f;

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
    density_value = 0.5f;
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
    shader.SetFloat("isoLevel", density_value);
}

void GUI::CreateGUI(double fps, Texture& noise3DTexture, const std::unique_ptr<Box>& box) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Scene");

        std::string fpsTitle = "FPS: " + std::to_string(int(fps));
        ImGui::Text("%s", fpsTitle.c_str());

        ImGui::Text("Main algorithm:");
        ImGui::SliderFloat("IsoLevel", &density_value, 0.0f, 1.0f);

        ImGui::Text("Noise:");
        ImGui::SliderFloat("Frequency", &box->frequency, 0.0f, 2.0f);
        ImGui::SliderFloat("Amplitude", &box->amplitude, 0.0f, 2.0f);

        if (ImGui::Button("Apply adjustments"))
        {
            box->FillGrid();

            noise3DTexture.Update(
                &box->grid[0][0][0],
                Box::GRID_X,
                Box::GRID_Y,
                Box::GRID_X,
                GL_RED,
                GL_FLOAT
            );
        }

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
