#include "Launcher.h"

Camera* Launcher::pCamera = nullptr;
glm::vec3 Launcher::rgbColorA = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 Launcher::rgbColorB = glm::vec3(1.0f, 0.0f, 0.0f);
bool Launcher::rotate = true;

void Launcher::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (pCamera != nullptr) {
		pCamera->ProcessScroll((float)yoffset);
	}
}

void Launcher::Load(GLFWwindow* window, GLfloat vertices[], GLsizeiptr vertSize, Shader& shaderProgram) {
	pCamera = new Camera(1920, 1080, glm::vec3(0.0f, 0.0f, 0.0f));

	// Регистрируем коллбэк в GLFW
	glfwSetScrollCallback(window, scroll_callback);

	VAO1 = new VAO();
	VAO1->Bind();  // Активация VAO для записи последующих конфигураций буферов

	// Выделение VRAM и аллокация данных (копирование массивов в видеопамять)
	VBO1 = new VBO(vertices, vertSize); // Аллокация GL_ARRAY_BUFFER

	// Описание макета данных (Layout) для вершинных атрибутов внутри VAO:
	// Атрибут 0 (Координаты): компонентность 3 (vec3), тип float, шаг 24 байта, смещение 0
	VAO1->LinkAttrib(*VBO1, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

	VAO1->Unbind(); // Сброс состояния VAO (защита от непреднамеренной мутации стейта)
	VBO1->Unbind(); // Развязка GL_ARRAY_BUFFER

	const int teapotW = 256;
	const int teapotH = 256;
	const int teapotD = 124;

	std::vector<uint8_t> rawBuffer(teapotW * teapotH * teapotD);

	//FILE NAMES:
	//mri_ventricles_256x256x124_uint8.raw HEAD MRI
	//boston_teapot_256x256x178_uint8.raw Teapot MRI
	//vis_male_128x256x256_uint8.raw Fun male head
	//bonsai_256x256x256_uint8.raw Bonsai tree
	//my_spine_volume.raw 1024*1024*29
	//ircad_patient_1.raw 512*512*111

	std::ifstream file("mri_ventricles_256x256x124_uint8.raw", std::ios::binary);
	
	/*if (!file.is_open()) {
		std::cerr << "КРИТИЧЕСКАЯ ОШИБКА: Не удалось открыть файл" << std::endl;
		return -1;
	}*/

	file.read(reinterpret_cast<char*>(rawBuffer.data()), rawBuffer.size());
	file.close();

	const int targetSize = 256;
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

	settings.InitGUI(window);
}

void Launcher::UIRender(double fps) {
	settings.UIInputs();
	settings.CreateGUI(fps, *teapotTexture, rgbColorA, rgbColorB, rotate);
}

void Launcher::CameraRender(GLFWwindow* window, float deltaTime) {
	pCamera->Inputs(window, deltaTime, rotate);
	pCamera->UpdateMatrix(45.0f, 0.1f, 100.0f);
}

void Launcher::Render(Shader& shaderProgram) {
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

void Launcher::UIEnd(Shader& shaderProgram) {
	settings.Render();
	settings.UniformValues(shaderProgram);
}

Launcher::~Launcher() {
	teapotTexture->Delete();
	normalTexture->Delete();
	VAO1->Delete();
	VBO1->Delete();
}