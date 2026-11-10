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

void Engine::Load(GLFWwindow* window, Shader& shaderProgram, VAO& vao, VBO& vbo, const char path[], const int sizeX, const int sizeY, const int sizeZ) {
	pCamera = new Camera(1920, 1080, glm::vec3(0.0f, 0.0f, 0.0f));

	glfwSetScrollCallback(window, scroll_callback);

	VAO1 = &vao;
	VBO1 = &vbo;

	std::vector<uint8_t> rawBuffer(sizeX * sizeY * sizeZ);

	std::ifstream file(path, std::ios::binary);

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
		nullptr,
		targetSize,
		targetSize,
		targetSize,
		GL_RGBA8,
		GL_UNSIGNED_BYTE,
		GL_TEXTURE1
	);

	ComputeShader computeShader("normal_map.comp");
	computeShader.Activate();

	glBindImageTexture(0, teapotTexture->ID, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);

	glBindImageTexture(1, normalTexture->ID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

	glDispatchCompute(targetSize / 8, targetSize / 8, targetSize / 8);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	normalTexture->texIUnit(shaderProgram, "uNormal", 1);
	
	settings.InitGUI(window);
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

	VAO1->Bind();

	shaderProgram.SetVec3("camera_position", pCamera->Position.x, pCamera->Position.y, pCamera->Position.z);

	shaderProgram.SetVec3("camForward", pCamera->camForward.x, pCamera->camForward.y, pCamera->camForward.z);
	shaderProgram.SetVec3("camRight", pCamera->camRight.x, pCamera->camRight.y, pCamera->camRight.z);
	shaderProgram.SetVec3("camUp", pCamera->camUp.x, pCamera->camUp.y, pCamera->camUp.z);

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