#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include<glad/glad.h>
#include"VBO.h" // Подключаем класс VBO, так как он используется в методе LinkVBO

class VAO {
public:
	GLuint ID; // Уникальный номер (паспорт) этого VAO на видеокарте
	VAO(); // Конструктор для создания объекта

	// Метод для связывания буфера вершин (VBO) с настройками этого VAO
	void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stribe, void* offset);
	void Bind(); // Включить этот VAO
	void Unbind(); // Выключить (отвязать) этот VAO
	void Delete(); // Удалить VAO из памяти видеокарты
};

#endif