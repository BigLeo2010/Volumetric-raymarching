#ifndef LAUNCHER_H
#define LAUNCHER_H

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<string>
#include<fstream>
#include<sstream>
#include<cerrno>

#include"shaderClass.h"
#include"Texture.h"
#include"VAO.h"
#include"VBO.h"
#include"Camera.h"
#include"Box.h"
#include"LauncherGUI.h"
#include"NormalGeneration.h"

class Launcher {
private:
	VAO* VAO1;
	VBO* VBO1;
public:
	int WIDTH = 800;
	int HEIGHT = 600;

	LauncherGUI settings;

	void Load(GLFWwindow* window, Shader& shaderProgram, VAO& vao, VBO& vbo);
	void UIRender(bool& isActive);
	void Render(Shader& shaderProgram);
	void UIEnd(Shader& shaderProgram);
};

#endif
