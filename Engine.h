#ifndef ENGINE_H
#define ENGINE_H

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
#include"GUI.h"
#include"NormalGeneration.h"

class Engine {
private:
	static Camera* pCamera;
	int WIDTH = 800;
	int HEIGHT = 600;

	VAO* VAO1;
	VBO* VBO1;

	Texture* teapotTexture;
	Texture* normalTexture;

	GUI settings;

	static glm::vec3 rgbColorA;
	static glm::vec3 rgbColorB;
	static bool rotate;
public:
	bool isActive = false;

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void Load(GLFWwindow* window, Shader& shaderProgram, VAO& vao, VBO& vbo);
	void UIRender(double fps);
	void CameraRender(GLFWwindow* window, float deltaTime);
	void Render(Shader& shaderProgram);
	void UIEnd(Shader& shaderProgram);
	~Engine();
};

#endif