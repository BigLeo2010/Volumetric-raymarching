#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include <glad/glad.h>
#include "shaderClass.h"
#include <stb/stb_image.h>

class Texture {
public:
	GLuint ID;
	GLenum type;
	GLenum slot;

	Texture(const void* data, int width, int height, int depth, GLenum format, GLenum pixelType, GLenum slot);
	Texture(const std::string& folderPath, const std::string& prefix, GLenum slot);

	void texIUnit(Shader& shader, const char* uniform, GLuint unit);
	void Update(const void* data, int width, int height, int depth, GLenum format, GLenum pixelType);

	void Bind();
	void Unbind();
	void Delete();
};

#endif
