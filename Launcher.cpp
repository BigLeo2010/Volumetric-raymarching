#include "Launcher.h"

void Launcher::Load(GLFWwindow* window, Shader& shaderProgram, VAO& vao, VBO& vbo) {
	VAO1 = &vao;
	VBO1 = &vbo;

	settings.InitGUI(window);
}

void Launcher::UIRender() {
	settings.UIInputs();
	settings.CreateGUI();
}

void Launcher::Render(Shader& shaderProgram) {
	VAO1->Bind(); // Контекстная активация сконфигурированных вершинных атрибутов
}

void Launcher::UIEnd(Shader& shaderProgram) {
	settings.Render();
	settings.UniformValues(shaderProgram);
}

Launcher::~Launcher() {
	VAO1->Delete();
	VBO1->Delete();
}