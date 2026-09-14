#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

// Разрешаем использование экспериментальных функций GLM (нужно для rotate_vector и vector_angle)
#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "shaderClass.h"

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 cameraMatrix = glm::mat4(1.0f);

    glm::vec3 camForward;
    glm::vec3 camRight;
    glm::vec3 camUp;

    int width;
    int height;

    double lastX = 0.0;
    double lastY = 0.0;

    float radius = 8.0f;
    float minRadius = 1.0f;
    float maxRadius = 20.0f;
    float yaw = 140.0f;
    float pitch = 23.0f;
    float sensitivity = 0.5f;

    bool firstClick = true;

    Camera(int width, int height, glm::vec3 target);

    void UpdateMatrix(float FOVdeg, float nearPlane, float farPlane);
    void Matrix(Shader& shader, const char* uniform);
    void Inputs(GLFWwindow* window, float deltaTime, bool canRotate);

    void ProcessScroll(float yoffset);
};

#endif