#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

// Разрешаем использование экспериментальных функций GLM (нужно для rotate_vector и vector_angle)
#define GLM_ENABLE_EXPERIMENTAL

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include"shaderClass.h"

class Camera {
public:
	// Основные векторы пространства камеры
	glm::vec3 Position;                             // Позиция камеры в мировых координатах
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f); // Направление взгляда (по умолчанию вперед по оси -Z)
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);     // Вектор "верх" для камеры (обычно ось +Y)
	glm::mat4 cameraMatrix = glm::mat4(1.0f);

	// Флаг первого клика (нужен, чтобы избежать резкого рывка камеры при зажатии мыши)
	bool firstClick = true;

	// Размеры окна (используются для расчета пропорций экрана и центрирования курсора)
	int width;
	int height;

	// Параметры движения
	float speed = 2.5f;         // Скорость перемещения камеры
	float sensitivity = 0.005f; // Чувствительность мыши (в радианах на пиксель)

	// Конструктор класса камеры
	Camera(int width, int height, glm::vec3 position);

	void UpdateMatrix(float FOVdeg, float nearPlane, float farPlane);

	void Matrix(Shader& shader, const char* uniform);

	// Обрабатывает ввод с клавиатуры и мыши каждую итерацию игрового цикла
	void Inputs(GLFWwindow* window, float deltaTime);
};

#endif