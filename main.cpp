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

	Camera camera(WIDTH, HEIGHT, glm::vec3(0.0f, 0.0f, 6.0f));

	// Инициализация графического конвейера (компиляция и линковка шейдеров)
	Shader shaderProgram("default.vert", "default.frag");

	VAO VAO1;     // Генерация Vertex Array Object (хранилище состояний вершинных атрибутов)
	VAO1.Bind();  // Активация VAO для записи последующих конфигураций буферов

	// Выделение VRAM и аллокация данных (копирование массивов в видеопамять)
	VBO VBO1(vertices, sizeof(vertices)); // Аллокация GL_ARRAY_BUFFER

	// Описание макета данных (Layout) для вершинных атрибутов внутри VAO:
	// Атрибут 0 (Координаты): компонентность 3 (vec3), тип float, шаг 24 байта, смещение 0
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

	VAO1.Unbind(); // Сброс состояния VAO (защита от непреднамеренной мутации стейта)
	VBO1.Unbind(); // Развязка GL_ARRAY_BUFFER

	double lastTime = glfwGetTime();
	double lastTimeFPS = glfwGetTime();
	int nbFrames = 0;
	double fps = 0;

	auto box = std::make_unique<Box>();
	box->FillGrid();

	const int teapotW = 256;
	const int teapotH = 256;
	const int teapotD = 124;

	std::vector<uint8_t> rawBuffer(teapotW * teapotH * teapotD);

	//FILE NAMES:
	//mri_ventricles_256x256x124_uint8.raw HEAD MRI
	//boston_teapot_256x256x178_uint8.raw Teapot MRI
	//vis_male_128x256x256_uint8.raw Fun male head
	//bonsai_256x256x256_uint8.raw Bonsai tree

	std::ifstream file("mri_ventricles_256x256x124_uint8.raw", std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "КРИТИЧЕСКАЯ ОШИБКА: Не удалось открыть файл" << std::endl;
		return -1;
	}
	file.read(reinterpret_cast<char*>(rawBuffer.data()), rawBuffer.size());
	file.close();

	const int targetSize = 256;
	std::vector<uint8_t> cubeBuffer(targetSize * targetSize * targetSize, 0);

	std::copy(rawBuffer.begin(), rawBuffer.end(), cubeBuffer.begin());

	Texture teapotTexture(
		cubeBuffer.data(), 
		targetSize,
		targetSize,
		targetSize,
		GL_RED,
		GL_UNSIGNED_BYTE,
		GL_TEXTURE0
	);

	teapotTexture.texIUnit(shaderProgram, "uNoise", 0);

	Texture normalTexture(
		NormalGeneration::GenerateNormalMap(targetSize, cubeBuffer).data(),
		targetSize,
		targetSize,
		targetSize,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		GL_TEXTURE1
	);

	normalTexture.texIUnit(shaderProgram, "uNormal", 1);


	GUI settings;
	settings.InitGUI(window);

	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();

		settings.UIInputs();
		settings.CreateGUI(fps, teapotTexture, box);

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

		camera.Inputs(window, deltaTime);
		camera.UpdateMatrix(45.0f, 0.1f, 100.0f);

		shaderProgram.Activate(); // Инжект шейдерной программы в текущий пайплайн

		camera.Matrix(shaderProgram, "camMatrix");

		teapotTexture.Bind();
		normalTexture.Bind();

		VAO1.Bind(); // Контекстная активация сконфигурированных вершинных атрибутов

		shaderProgram.SetFloat("time", (float)glfwGetTime());
		shaderProgram.SetVec3("camera_position", camera.Position.x, camera.Position.y, camera.Position.z);

		glm::vec3 forward = camera.Orientation;
		glm::vec3 right = glm::normalize(glm::cross(forward, camera.Up));
		glm::vec3 up = glm::cross(right, forward);

		shaderProgram.SetVec3("camForward", forward.x, forward.y, forward.z);
		shaderProgram.SetVec3("camRight", right.x, right.y, right.z);
		shaderProgram.SetVec3("camUp", up.x, up.y, up.z);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		settings.Render();
		settings.UniformValues(shaderProgram);

		glfwSwapBuffers(window);
	}

	teapotTexture.Delete();
	normalTexture.Delete();
	VAO1.Delete();
	VBO1.Delete();
	shaderProgram.Delete();

	glfwDestroyWindow(window); // Уничтожение дескриптора окна
	glfwTerminate();           // Корректное завершение работы подсистемы GLFW

	return 0;
}