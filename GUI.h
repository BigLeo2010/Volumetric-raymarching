#ifndef GUI_H
#define GUI_H

#include "shaderClass.h"
#include "Texture.h"
#include "Box.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <memory>

class GUI {
public:
	ImGuiIO* io = nullptr;
	GLFWwindow* windowGUI = nullptr;
	static float density_value;

	GUI();
	void InitGUI(GLFWwindow* window);
	void UIInputs();
	void UniformValues(Shader& shader);
	void CreateGUI(double fps, Texture& noise3DTexture, const std::unique_ptr<Box>& box);
	void Render();
	~GUI();
};

#endif
