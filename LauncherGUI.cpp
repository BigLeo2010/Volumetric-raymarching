#include "LauncherGUI.h"
#include <string>
#include "portable-file-dialogs.h"

LauncherGUI::LauncherGUI() {}

void LauncherGUI::InitGUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    io = &ImGui::GetIO();

    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.40f, 0.80f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.50f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.35f, 0.70f, 1.00f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    windowGUI = window;
}

void LauncherGUI::UIInputs() {
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

void LauncherGUI::UniformValues(Shader& shader) {

}

void LauncherGUI::CreateGUI(bool& isActive) {
    if (!windowGUI || !io) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(275, 250), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(250,100), ImGuiCond_Always);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar |       // Убираем заголовок
        ImGuiWindowFlags_NoResize |         // Запрещаем менять размер
        ImGuiWindowFlags_NoMove |           // Запрещаем двигать окно
        ImGuiWindowFlags_NoScrollbar |      // Убираем скроллбары
        ImGuiWindowFlags_NoBackground |     // Полностью прозрачный фон
        ImGuiWindowFlags_NoCollapse |       // Запрещаем сворачивать
        ImGuiWindowFlags_NoSavedSettings;   // Игнорируем imgui.ini

    if (ImGui::Begin("LauncherContainer", nullptr, window_flags))
    {
        // Буфер для хранения пути к файлу (сохраняет состояние между кадрами)
        static char filePath[512] = "";

        // Поле ввода пути (займет большую часть ширины)
        ImGui::PushItemWidth(200.0f);
        ImGui::InputText("##PathInput", filePath, IM_ARRAYSIZE(filePath));
        ImGui::PopItemWidth();

        ImGui::SameLine();

        // Кнопка вызова проводника
        if (ImGui::Button("Обзор...")) {
            // Вызываем кроссплатформенный диалог выбора файла через portable-file-dialogs
            auto selection = pfd::open_file("Выберите файл", ".",
                { "Все файлы", "*" }).result();

            // Если пользователь выбрал файл и не закрыл окно крестиком
            if (!selection.empty()) {
                // Копируем путь в наш char-массив безопасным методом
                strcpy_s(filePath, selection[0].c_str());
            }
        }

        ImGui::Spacing(); // Небольшой отступ перед кнопкой СТАРТ

        if (ImGui::Button("START", ImVec2(-1, -1))) {
            isActive = true;
        }
    }

    ImGui::End();
}


void LauncherGUI::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

LauncherGUI::~LauncherGUI() {
    if (windowGUI) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
