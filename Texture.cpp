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

// Конструктор специально для загрузки массива кадров STBN с диска
Texture::Texture(const std::string& folderPath, const std::string& prefix, GLenum slot) {
    // 1. Устанавливаем тип Текстурного Массива
    type = GL_TEXTURE_2D_ARRAY;
    this->slot = slot;

    const int width = 128;
    const int height = 128;
    const int layers = 64; // В архиве STBN ровно 64 кадра

    glGenTextures(1, &ID);
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

    // 2. ВАЖНО: Для синего шума используем только GL_NEAREST (без сглаживания)
    // Иначе математические свойства шума сломаются
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Зацикливаем текстуру, чтобы % 128 в шейдере работал автоматически при GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 3. Выделяем неизменяемую память на GPU под 64 слоя (1 канал, 8 бит на пиксель)
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, width, height, layers);

    // 4. Поочередно загружаем файлы png
    stbi_set_flip_vertically_on_load(false);

    for (int i = 0; i < layers; ++i) {
        // Собираем путь, например: "textures/STBN_scalar_0.png"
        std::string fileName = folderPath + "/" + prefix + "_" + std::to_string(i) + ".png";

        int imgWidth, imgHeight, imgChannels;
        // Загружаем картинку, форсируя 1 канал (GL_RED)
        unsigned char* data = stbi_load(fileName.c_str(), &imgWidth, &imgHeight, &imgChannels, 1);

        if (!data) {
            std::cerr << "STBN Error: Не удалось загрузить файл: " << fileName << std::endl;
            continue;
        }

        // Загружаем этот кадр в массив на слой номер i
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
            0, 0, i,             // Смещение по X, Y и индекс слоя
            width, height, 1,    // Размер подгружаемого куска (1 слой)
            GL_RED, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture::Update(const void* data, int width, int height, int depth, GLenum format, GLenum pixelType) {
	// Активируем текстурный слот и привязываем текстуру
	glActiveTexture(slot);
	glBindTexture(type, ID);

	// Выравнивание строк данных в памяти (как в конструкторе)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Обновляем данные всей текстуры (начиная с офсета 0, 0, 0)
	glTexSubImage3D(type, 0, 0, 0, 0, width, height, depth, format, pixelType, data);

	// Отвязываем текстуру
	glBindTexture(type, 0);
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
