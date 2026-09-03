#include"VBO.h"

// Конструктор класса
VBO::VBO(GLfloat* vertices, GLsizeiptr size) {
	glGenBuffers(1, &ID); // Генерируем буфер и записываем его уникальный ID
	glBindBuffer(GL_ARRAY_BUFFER, ID); // Привязываем VBO как буфер вершинного массива
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW); // Отправляем координаты вершин на видеокарту
}

// Метод для активации VBO
void VBO::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, ID); // Делаем этот буфер вершин активным
}

// Метод для деактивации VBO
void VBO::Unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Отвязываем текущий буфер вершин (0 — сброс)
}

// Метод для очистки памяти
void VBO::Delete() {
	glDeleteBuffers(1, &ID); // Удаляем буфер вершин с видеокарты
}