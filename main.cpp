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
#include"Engine.h"

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

void change_window_size(GLFWwindow* window, int width, int height) {
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (monitor) {
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		if (mode) {
			if (width >= mode->width || height >= mode->height) {
				glfwMaximizeWindow(window);

				int display_w, display_h;
				glfwGetFramebufferSize(window, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);
				return;
			}
		}
	}

	glfwSetWindowSize(window, width, height);
	glViewport(0, 0, width, height);

	if (monitor) {
		int monitor_x, monitor_y, monitor_width, monitor_height;
		glfwGetMonitorWorkarea(monitor, &monitor_x, &monitor_y, &monitor_width, &monitor_height);

		int frame_left, frame_top, frame_right, frame_bottom;
		glfwGetWindowFrameSize(window, &frame_left, &frame_top, &frame_right, &frame_bottom);

		int full_window_width = width + frame_left + frame_right;
		int full_window_height = height + frame_top + frame_bottom;

		int new_x = monitor_x + (monitor_width - full_window_width) / 2;
		int new_y = monitor_y + (monitor_height - full_window_height) / 2;

		glfwSetWindowPos(window, new_x, new_y);
	}
}

struct Metadata {
	int width = 0;
	int height = 0;
	int depth = 0;
	std::string dataType;
	std::string filePath;
};


int main() 
{
	system("chcp 1251 > nul"); // Локализация вывода консоли (кодовая страница Windows-1251)

	glfwInit(); // Инициализация подсистемы GLFW

	// Конфигурация контекста дескриптора окна (OpenGL 4.3 Core Profile)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Отсечение deprecated-функционала

	GLfloat backgroundColor[] = { 45.0f/255.0f, 45.0f / 255.0f, 45.0f / 255.0f }; // Нормализованные RGBA значения цвета очистки

	Engine engine;

	// Инстанцирование объекта окна и создание ассоциированного контекста OpenGL
	GLFWwindow* window = glfwCreateWindow(engine.WIDTH, engine.HEIGHT, "Volumetric Raymarching", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create a window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window); // Привязка контекста OpenGL к текущему потоку выполнения

	gladLoadGL(); // Динамическая загрузка указателей на функции API OpenGL через GLAD

	glViewport(0, 0, engine.WIDTH, engine.HEIGHT);


	/* ВОТ ЭТУ ХУЙНЮ НЕ ТРОГАТЬ */

	// Инициализация графического конвейера (компиляция и линковка шейдеров)
	Shader shaderProgram("default.vert", "default.frag");

	VAO VAO1;
	VBO VBO1(vertices, sizeof(vertices));

	VAO1.Bind();  // Активация VAO для записи последующих конфигураций буферов

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

	VAO1.Unbind(); // Сброс состояния VAO (защита от непреднамеренной мутации стейта)
	VBO1.Unbind(); // Развязка GL_ARRAY_BUFFER

	/* ВСЕ, МОЖНО ТРОГАТЬ ДАЛЬШЕ */

	// volume config

	std::ifstream file("volume_config.txt");

	if (!file.is_open()) {
		std::cerr << "Ошибка: Не удалось открыть конфигурационный файл!" << std::endl;
		return 1;
	}

	Metadata data;

	if (file >> data.width &&
		file >> data.height &&
		file >> data.depth &&
		file >> data.dataType) {

		file >> std::ws;
		std::getline(file, data.filePath);
	}
	else {
		std::cerr << "Ошибка: Неверный формат данных в файле!" << std::endl;
	}

	file.close();

	// main volume

	//FILE NAMES:
	//mri_ventricles_256x256x124_uint8.raw HEAD MRI
	//boston_teapot_256x256x178_uint8.raw Teapot MRI
	//vis_male_128x256x256_uint8.raw Fun male head
	//bonsai_256x256x256_uint8.raw Bonsai tree
	//my_spine_volume.raw 1024*1024*29
	//ircad_patient_1.raw 512*512*111

	std::string path = "ircad_patient_1.raw";
	engine.Load(window, shaderProgram, VAO1, VBO1, path.c_str(), 512, 512, 111);

	Texture stbnNoise("STBN", "stbn_scalar_2Dx1Dx1D_128x128x64x1", GL_TEXTURE2);

	double lastTime = glfwGetTime();
	double lastTimeFPS = glfwGetTime();
	int nbFrames = 0;
	double fps = 0;

	int frameCounter = 0;

	bool engineHasLoaded = false;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		engine.UIRender(fps);

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

		engine.CameraRender(window, deltaTime);

		shaderProgram.Activate(); // Инжект шейдерной программы в текущий пайплайн

		engine.Render(shaderProgram);

		stbnNoise.Bind();
		//stbnNoise.texIUnit(shaderProgram, "stbnTexture", 1);

		shaderProgram.SetInt("frameIndex", frameCounter);
		shaderProgram.SetFloat("time", (float)glfwGetTime());

		glDrawArrays(GL_TRIANGLES, 0, 6);

		engine.UIEnd(shaderProgram);

		glfwSwapBuffers(window);
		frameCounter++;
	}
	
	shaderProgram.Delete();
	stbnNoise.Delete();

	glfwDestroyWindow(window); // Уничтожение дескриптора окна
	glfwTerminate();           // Корректное завершение работы подсистемы GLFW

	return 0;
}