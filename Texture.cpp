#include "Texture.h"

// Конструктор для создания 3D текстуры из трехмерного массива данных в памяти
Texture::Texture(const void* data, int width, int height, int depth, GLenum format, GLenum pixelType, GLenum slot) {
	type = GL_TEXTURE_3D;
	this->slot = slot;

	glGenTextures(1, &ID);
	glActiveTexture(slot);
	glBindTexture(GL_TEXTURE_3D, ID);

	// Обязательно GL_LINEAR для плавного сглаживания float-значений шума
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLenum internalFormat = (format == GL_RED) ? GL_R32F : format;

	// Загружаем 3D-массив float'ов
	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, pixelType, data);

	glBindTexture(GL_TEXTURE_3D, 0);
}


void Texture::texIUnit(Shader& shader, const char* uniform, GLuint unit) {
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(texUni, unit);
}

void Texture::Bind() {
	// Перед привязкой обязательно активируем правильный слот, сохраненный в конструкторе
	glActiveTexture(slot);
	glBindTexture(type, ID);
}

void Texture::Unbind() {
	glActiveTexture(slot);
	glBindTexture(type, 0);
}

void Texture::Delete() {
	glDeleteTextures(1, &ID);
}
