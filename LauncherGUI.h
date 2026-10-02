#ifndef LAUNCHER_GUI_H
#define LAUNCHER_GUI_H

#include "shaderClass.h"
#include "Texture.h"
#include "Box.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <glm/glm.hpp>

class LauncherGUI {
public:
	ImGuiIO* io = nullptr;
	GLFWwindow* windowGUI = nullptr;
	static char filePath[512];

	LauncherGUI();
	void InitGUI(GLFWwindow* window);
	void UIInputs();
	void UniformValues(Shader& shader);
	void CreateGUI(bool& isActive);
	void Render();
	~LauncherGUI();
};

#endif
