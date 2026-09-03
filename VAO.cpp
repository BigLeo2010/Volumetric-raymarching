#include"VAO.h"

// Конструктор класса
VAO::VAO() {
	glGenVertexArrays(1, &ID); // Генерируем VAO и записываем его ID
}

// Метод для связывания VBO с этим VAO
void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stribe, void* offset) {
	VBO.Bind(); // Включаем нужный буфер вершин (VBO)

	// Настраиваем, как видеокарта должна читать данные (размер шага изменен на 0, так как данные идут вплотную)
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stribe, offset);

	glEnableVertexAttribArray(layout); // Включаем этот атрибут по его номеру (layout)
	VBO.Unbind(); // Отвязываем VBO, настройки уже сохранились внутри VAO
}

// Метод для активации VAO
void VAO::Bind() {
	glBindVertexArray(ID); // Делаем этот VAO текущим (активным)
}

// Метод для деактивации VAO
void VAO::Unbind() {
	glBindVertexArray(0); // Отвязываем текущий VAO
}

// Метод для очистки памяти
void VAO::Delete() {
	glDeleteVertexArrays(1, &ID); // Удаляем VAO с видеокарты
}