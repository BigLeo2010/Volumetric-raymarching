#include "Engine.h"
#include <algorithm>

Camera* Engine::pCamera = nullptr;
glm::vec3 Engine::rgbColorA = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 Engine::rgbColorB = glm::vec3(1.0f, 0.0f, 0.0f);
bool Engine::rotate = true;

void Engine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (pCamera != nullptr) {
		pCamera->ProcessScroll((float)yoffset);
	}
}

void Engine::Load(GLFWwindow* window, Shader& shaderProgram, VAO& vao, VBO& vbo, char path[], const int sizeX, const int sizeY, const int sizeZ) {
	pCamera = new Camera(1920, 1080, glm::vec3(0.0f, 0.0f, 0.0f));

	// Регистрируем коллбэк в GLFW
	glfwSetScrollCallback(window, scroll_callback);

	VAO1 = &vao;
	VBO1 = &vbo;

	std::vector<uint8_t> rawBuffer(sizeX * sizeY * sizeZ);

	//FILE NAMES:
	//mri_ventricles_256x256x124_uint8.raw HEAD MRI
	//boston_teapot_256x256x178_uint8.raw Teapot MRI
	//vis_male_128x256x256_uint8.raw Fun male head
	//bonsai_256x256x256_uint8.raw Bonsai tree
	//my_spine_volume.raw 1024*1024*29
	//ircad_patient_1.raw 512*512*111

	std::ifstream file(path, std::ios::binary);

	/*if (!file.is_open()) {
		std::cerr << "КРИТИЧЕСКАЯ ОШИБКА: Не удалось открыть файл" << std::endl;
		return -1;
	}*/

	file.read(reinterpret_cast<char*>(rawBuffer.data()), rawBuffer.size());
	file.close();

	const int targetSize = std::max({sizeX, sizeY, sizeZ});
	std::vector<uint8_t> cubeBuffer(targetSize * targetSize * targetSize, 0);

	std::copy(rawBuffer.begin(), rawBuffer.end(), cubeBuffer.begin());

	teapotTexture = new Texture(
		cubeBuffer.data(),
		targetSize,
		targetSize,
		targetSize,
		GL_RED,
		GL_UNSIGNED_BYTE,
		GL_TEXTURE0
	);

	teapotTexture->texIUnit(shaderProgram, "uNoise", 0);

	normalTexture = new Texture(
		NormalGeneration::GenerateNormalMap(targetSize, cubeBuffer).data(),
		targetSize,
		targetSize,
		targetSize,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		GL_TEXTURE1
	);

	normalTexture->texIUnit(shaderProgram, "uNormal", 1);
	
	//settings.InitGUI(window);
}

void Engine::UIRender(double fps) {
	settings.UIInputs();
	settings.CreateGUI(fps, *teapotTexture, rgbColorA, rgbColorB, rotate);
}

void Engine::CameraRender(GLFWwindow* window, float deltaTime) {
	pCamera->Inputs(window, deltaTime, rotate);
	pCamera->UpdateMatrix(45.0f, 0.1f, 100.0f);
}

void Engine::Render(Shader& shaderProgram) {
	pCamera->Matrix(shaderProgram, "camMatrix");

	teapotTexture->Bind();
	normalTexture->Bind();

	VAO1->Bind(); // Контекстная активация сконфигурированных вершинных атрибутов

	// Передаем позицию камеры в шейдер
	shaderProgram.SetVec3("camera_position", pCamera->Position.x, pCamera->Position.y, pCamera->Position.z);

	// Передаем векторы направления камеры (Forward, Right, Up)
	shaderProgram.SetVec3("camForward", pCamera->camForward.x, pCamera->camForward.y, pCamera->camForward.z);
	shaderProgram.SetVec3("camRight", pCamera->camRight.x, pCamera->camRight.y, pCamera->camRight.z);
	shaderProgram.SetVec3("camUp", pCamera->camUp.x, pCamera->camUp.y, pCamera->camUp.z);

	// Передаем цвета (они статические члены Launcher, берутся напрямую)
	shaderProgram.SetVec3("rgbColorA", rgbColorA.x, rgbColorA.y, rgbColorA.z);
	shaderProgram.SetVec3("rgbColorB", rgbColorB.x, rgbColorB.y, rgbColorB.z);

}

void Engine::UIEnd(Shader& shaderProgram) {
	settings.Render();
	settings.UniformValues(shaderProgram);
}

Engine::~Engine() {
	if (teapotTexture) {
		teapotTexture->Delete();
		delete teapotTexture;
	}
	if (normalTexture) {
		normalTexture->Delete();
		delete normalTexture;
	}

	if (pCamera) {
		delete pCamera;
		pCamera = nullptr;
	}
}