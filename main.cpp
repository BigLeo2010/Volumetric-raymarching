#ifdef _WIN32
#include <windows.h>
extern "C" {
	// Для видеокарт NVIDIA (Optimus)
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

	// Для видеокарт AMD/Radeon
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

// САЙТ С РАЗНЫМИ ШУМАМИ
// http://klacansky.com/open-scivis-datasets/
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
#include"Launcher.h"

GLfloat vertices[] = {
	// Первый треугольник
	-1.0f, -1.0f, 0.0f,  // 1. Низ-лево
	 1.0f, -1.0f, 0.0f,  // 2. Низ-право
	 1.0f,  1.0f, 0.0f,  // 3. Верх-право

	 // Второй треугольник
	  1.0f,  1.0f, 0.0f,  // 3. Верх-право
	 -1.0f,  1.0f, 0.0f,  // 4. Верх-лево
	 -1.0f, -1.0f, 0.0f   // 1. Низ-лево
};

int main() 
{
	system("chcp 1251 > nul"); // Локализация вывода консоли (кодовая страница Windows-1251)

	glfwInit(); // Инициализация подсистемы GLFW

	// Конфигурация контекста дескриптора окна (OpenGL 3.3 Core Profile)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Отсечение deprecated-функционала

	int WIDTH = 1920;
	int HEIGHT = 1080;

	GLfloat backgroundColor[] = { 45.0f/255.0f, 45.0f / 255.0f, 45.0f / 255.0f }; // Нормализованные RGBA значения цвета очистки

	// Инстанцирование объекта окна и создание ассоциированного контекста OpenGL
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Volumetric Raymarching", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create a window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window); // Привязка контекста OpenGL к текущему потоку выполнения

	gladLoadGL(); // Динамическая загрузка указателей на функции API OpenGL через GLAD

	glViewport(0, 0, WIDTH, HEIGHT);

	// Инициализация графического конвейера (компиляция и линковка шейдеров)
	Shader shaderProgram("default.vert", "default.frag");

	Launcher launcher;
	launcher.Load(window, vertices, sizeof(vertices), shaderProgram);

	double lastTime = glfwGetTime();
	double lastTimeFPS = glfwGetTime();
	int nbFrames = 0;
	double fps = 0;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		launcher.UIRender(fps);

		glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		double curTime = glfwGetTime();
		float deltaTime = (float)(curTime - lastTime);
		lastTime = curTime;

		nbFrames++;

		if (curTime - lastTimeFPS >= 1.0)
		{
			fps = double(nbFrames);
			nbFrames = 0;
			lastTimeFPS += 1.0;
		}

		launcher.CameraRender(window, deltaTime);

		shaderProgram.Activate(); // Инжект шейдерной программы в текущий пайплайн

		shaderProgram.SetFloat("time", (float)glfwGetTime());

		launcher.Render(shaderProgram);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		launcher.UIEnd(shaderProgram);

		glfwSwapBuffers(window);
	}
	
	shaderProgram.Delete();

	glfwDestroyWindow(window); // Уничтожение дескриптора окна
	glfwTerminate();           // Корректное завершение работы подсистемы GLFW

	return 0;
}