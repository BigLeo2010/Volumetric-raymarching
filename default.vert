#version 330 core // Указывает версию OpenGL (3.3) и профиль Core (современный, без устаревших функций)

// Входные данные (атрибуты вершин), которые передаются из C++ кода через VAO/VBO
layout (location = 0) in vec3 aPos;

out vec3 pos;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	pos = aPos;
}