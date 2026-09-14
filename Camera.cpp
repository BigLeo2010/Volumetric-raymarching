#include "Camera.h"

Camera::Camera(int width, int height, glm::vec3 target) {
	this->width = width;
	this->height = height;
	this->Target = target;

	UpdateMatrix(45.0f, 0.1f, 100.0f);
}

void Camera::UpdateMatrix(float FOVdeg, float nearPlane, float farPlane) {
	glm::vec3 offset;
	offset.x = radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	offset.y = radius * sin(glm::radians(pitch));
	offset.z = radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

	Position = Target + offset;

	camForward = glm::normalize(Target - Position);

	camRight = glm::normalize(glm::cross(camForward, Up));
	camUp = glm::normalize(glm::cross(camRight, camForward));

	glm::mat4 view = glm::lookAt(Position, Target, Up);
	glm::mat4 projection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane);

	cameraMatrix = projection * view;
}



void Camera::Matrix(Shader& shader, const char* uniform) {
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Inputs(GLFWwindow* window, float deltaTime, bool canRotate) {

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

#ifdef _WIN32
		// КОД ДЛЯ WINDOWS
		if (firstClick) {
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float offsetX = (float)(mouseX - (width / 2)) * sensitivity;
		float offsetY = (float)(-(height / 2) + mouseY) * sensitivity;

		yaw += offsetX;
		pitch += offsetY;

		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

		glfwSetCursorPos(window, (width / 2), (height / 2));
#else
		// КОД ДЛЯ LINUX/WSL
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		if (firstClick) {
			lastX = mouseX;
			lastY = mouseY;
			firstClick = false;
		}

		float offsetX = (float)(mouseX - lastX) * sensitivity;
		float offsetY = (float)(mouseY - lastY) * sensitivity;

		lastX = mouseX;
		lastY = mouseY;

		yaw += offsetX;
		pitch += offsetY;

		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;
#endif
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;

		if (canRotate) yaw += deltaTime * -20.0f;
	}
}


void Camera::ProcessScroll(float yoffset) {
	radius -= yoffset / 2;

	if (radius < minRadius) radius = minRadius;
	if (radius > maxRadius) radius = maxRadius;
}