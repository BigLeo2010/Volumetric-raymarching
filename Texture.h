#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include <glad/glad.h>
#include "shaderClass.h"

class Texture {
public:
	GLuint ID;
	GLenum type; // Всегда будет GL_TEXTURE_3D
	GLenum slot; // Хранит привязанный текстурный слот (например, GL_TEXTURE0)

	// Конструктор принимает указатель на 3D-массив байт, его размеры (ширину, высоту, глубину), 
	// формат (например, GL_RED для шума) и текстурный слот
	Texture(const void* data, int width, int height, int depth, GLenum format, GLenum pixelType, GLenum slot);

	// Привязка uniform-переменной шейдера к конкретному номеру слота
	void texIUnit(Shader& shader, const char* uniform, GLuint unit);
	void Update(const void* data, int width, int height, int depth, GLenum format, GLenum pixelType);

	void Bind();
	void Unbind();
	void Delete();
};

#endif
