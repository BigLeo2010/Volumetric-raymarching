#ifndef GUI_H
#define GUI_H

#include "shaderClass.h"
#include "Texture.h"
#include "Box.h"
#include "imgui.h"
#include "imgui_color_gradient.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <glm/glm.hpp>

class GUI {
public:
	ImGuiIO* io = nullptr;
	GLFWwindow* windowGUI = nullptr;
	static float isoMin;
	static float isoMax;

	static float clipX;
	static float clipY;
	static float clipZ;

	static bool smoothing;
	static bool use_dvr;

	static ImGradient gradient;
	static ImGradientMark* draggingMark;
	static ImGradientMark* selectedMark;
	static GLuint colorID;

	static void UpdateTransferFunctionTexture();

	GUI();
	void InitGUI(GLFWwindow* window);
	void UIInputs();
	void UniformValues(Shader& shader);
	void CreateGUI(double fps, Texture& noise3DTexture, bool& canRotate);
	void Render();
	~GUI();
};

#endif
