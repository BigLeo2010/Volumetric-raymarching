#include"Camera.h"

// Реализация конструктора: инициализируем позицию и размеры экрана
Camera::Camera(int width, int height, glm::vec3 position) {
	Position = position;
	Camera::width = width;
	Camera::height = height;
}

// Расчет и передача MVP-матрицы в шейдер
void Camera::UpdateMatrix(float FOVdeg, float nearPlane, float farPlane) {
	// Инициализируем единичные матрицы
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// lookAt создает матрицу вида на основе: где мы стоим, куда смотрим (Позиция + Направление) и где верх
	view = glm::lookAt(Position, Position + Orientation, Up);

	// perspective создает матрицу проекции. Приводим width и height к float, чтобы избежать целочисленного деления
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane);

	cameraMatrix = projection * view;
}

void Camera::Matrix(Shader& shader, const char* uniform) {
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

// Обработка клавиш и перемещения мыши (ИЗМЕНЕНО: Добавлен аргумент float deltaTime)
void Camera::Inputs(GLFWwindow* window, float deltaTime) {

	// Ускорение при зажатом Left Shift (спринт)
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		speed = 4.0f; // ИЗМЕНЕНО: Значения увеличены, так как теперь они умножаются на секунды
	}
	// Возврат к обычной скорости, если Shift отпущен
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		speed = 1.5f; // ИЗМЕНЕНО: Значения увеличены
	}

	// ДОБАВЛЕНО: Расчет скорости для текущего кадра на основе deltaTime
	float currentSpeed = speed * deltaTime;

	// --- ДВИЖЕНИЕ НА КЛАВИАТУРУ ---

	// Движение ВПЕРЕД (по вектору направления взгляда)
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		Position += currentSpeed * Orientation; // ИЗМЕНЕНО: Заменено на currentSpeed
	}
	// Движение ВЛЕВО (перпендикулярно взгляду и верху). Используем векторное произведение (cross product)
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		Position += currentSpeed * -glm::normalize(glm::cross(Orientation, Up)); // ИЗМЕНЕНО: Заменено на currentSpeed
	}
	// Движение НАЗАД (против вектора направления взгляда)
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		Position += currentSpeed * -Orientation; // ИЗМЕНЕНО: Заменено на currentSpeed
	}
	// Движение ВПРАВО (перпендикулярно взгляду и верху)
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		Position += currentSpeed * glm::normalize(glm::cross(Orientation, Up)); // ИЗМЕНЕНО: Заменено на currentSpeed
	}
	// Движение ВВЕРХ (строго по мировой оси Up)
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		Position += currentSpeed * Up; // ИЗМЕНЕНО: Заменено на currentSpeed
	}
	// Движение ВНИЗ (противно мировой оси Up)
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		Position += currentSpeed * -Up; // ИЗМЕНЕНО: Заменено на currentSpeed
	}

	// --- ПОВОРОТ КАМЕРЫ МЫШЬЮ (ПО ЗАЖАТИЮ ПКМ) ---

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		// Скрываем курсор мыши, чтобы он не мешал обзору
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Если это самый первый кадр клика, принудительно перемещаем мышь в центр, чтобы не было рывка камеры
		if (firstClick) {
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		// Переменные для хранения текущих координат курсора
		double mouseX;
		double mouseY;
		// Получаем позицию мыши от GLFW
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Вычисляем смещение мыши относительно центра экрана и умножаем на чувствительность
		// rotX отвечает за наклон ВВЕРХ-ВНИЗ (зависит от изменения по Y)
		// rotY отвечает за поворот ВЛЕВО-ВПРАВО (зависит от изменения по X)
		float rotX = sensitivity * (float)(mouseY - (height / 2));
		float rotY = sensitivity * (float)(mouseX - (width / 2));

		// Вычисляем гипотетическое новое направление взгляда по вертикали. 
		// Вращаем вокруг оси "вправо" (которая является перпендикуляром взгляда и верха)
		glm::vec3 newOrientation = glm::rotate(Orientation, -rotX, glm::normalize(glm::cross(Orientation, Up)));

		// Защита от переворота камеры «через голову» (Gimbal Lock)
		// Если угол между новым взглядом и осью Up (или -Up) меньше 5 градусов — запрещаем поворот
		if (!((glm::angle(newOrientation, Up) <= glm::radians(5.0f)) || (glm::angle(newOrientation, -Up) <= glm::radians(5.0f)))) {
			Orientation = newOrientation;
		}

		// Вращаем камеру по горизонтали вокруг глобальной оси Up (влево-вправо)
		Orientation = glm::rotate(Orientation, -rotY, Up);

		// Возвращаем курсор обратно в центр экрана для следующего кадра
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	// Если правая кнопка мыши отпущена
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		// Возвращаем стандартный курсор
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// Сбрасываем флаг, чтобы при следующем клике позиция мыши отцентрировалась корректно
		firstClick = true;
	}
}