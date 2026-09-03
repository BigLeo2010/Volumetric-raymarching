#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include<glad/glad.h>

class VBO {
public:
	GLuint ID; // Уникальный номер (паспорт) этого буфера вершин на видеокарте
	VBO(GLfloat* vertices, GLsizeiptr size); // Конструктор для создания и заполнения буфера координатами

	void Bind(); // Включить этот буфер вершин
	void Unbind(); // Выключить (отвязать) этот буфер вершин
	void Delete(); // Удалить буфер вершин из памяти видеокарты
};

#endif