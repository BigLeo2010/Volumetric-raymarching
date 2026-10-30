#include "Texture.h"

Texture::Texture(const void* data, int width, int height, int depth, GLenum format, GLenum pixelType, GLenum slot) {
	type = GL_TEXTURE_3D;
	this->slot = slot;

	glGenTextures(1, &ID);
	glActiveTexture(slot);
	glBindTexture(GL_TEXTURE_3D, ID);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLenum internalFormat = (format == GL_RED) ? GL_R32F : format;

	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, pixelType, data);

	glBindTexture(GL_TEXTURE_3D, 0);
}

Texture::Texture(const std::string& folderPath, const std::string& prefix, GLenum slot) {
    type = GL_TEXTURE_2D_ARRAY;
    this->slot = slot;

    const int width = 128;
    const int height = 128;
    const int layers = 64;

    glGenTextures(1, &ID);
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, width, height, layers);

    stbi_set_flip_vertically_on_load(false);

    for (int i = 0; i < layers; ++i) {
        std::string fileName = folderPath + "/" + prefix + "_" + std::to_string(i) + ".png";

        int imgWidth, imgHeight, imgChannels;
        unsigned char* data = stbi_load(fileName.c_str(), &imgWidth, &imgHeight, &imgChannels, 1);

        if (!data) {
            std::cerr << "STBN Error: Не удалось загрузить файл: " << fileName << std::endl;
            continue;
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
            0, 0, i,
            width, height, 1,
            GL_RED, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture::Update(const void* data, int width, int height, int depth, GLenum format, GLenum pixelType) {
	glActiveTexture(slot);
	glBindTexture(type, ID);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexSubImage3D(type, 0, 0, 0, 0, width, height, depth, format, pixelType, data);

	glBindTexture(type, 0);
}


void Texture::texIUnit(Shader& shader, const char* uniform, GLuint unit) {
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(texUni, unit);
}

void Texture::Bind() {
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
