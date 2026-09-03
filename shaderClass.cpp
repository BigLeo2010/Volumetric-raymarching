#include"shaderClass.h"

// Функция для быстрого чтения всего содержимого файла в бинарном режиме
std::string get_file_contents(const char* filename) {
	std::ifstream in(filename, std::ios::binary); // Открываем файл для чтения
	if (in) {
		std::string contents;
		in.seekg(0, std::ios::end); // Перемещаем указатель в самый конец файла
		contents.resize(in.tellg()); // Узнаем размер файла и выделяем столько же места в строке
		in.seekg(0, std::ios::beg); // Возвращаем указатель в начало файла
		in.read(&contents[0], contents.size()); // Считываем весь текст в строку одной командой
		in.close(); // Закрываем файл
		return(contents); // Возвращаем прочитанный текст
	}
	throw(errno); // Если файл не открылся, выбрасываем код ошибки системы
}

// Конструктор класса Shader, принимающий пути к файлам шейдеров
Shader::Shader(const char* vertexFile, const char* fragmentFile) {
	// Считываем код шейдеров из указанных файлов в обычные строки C++
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	// Переводим строки C++ в формат C-строк (const char*), который требует OpenGL
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Создаем и компилируем вершинный шейдер
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); //Создаем объект шейдера
	glShaderSource(vertexShader, 1, &vertexSource, NULL); //Привязываем текст
	glCompileShader(vertexShader); //Компилируем
	compileErrors(vertexShader, "VERTEX");

	// Создаем и компилируем фрагментный шейдер
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Создаем объект шейдера
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL); //Привязываем текст
	glCompileShader(fragmentShader); //Компилируем
	compileErrors(fragmentShader, "FRAGMENT");

	// Создаем общую шейдерную программу
	ID = glCreateProgram(); //Создаем программу и записываем её ID в переменную класса

	glAttachShader(ID, vertexShader); //Прикрепляем вершинный шейдер
	glAttachShader(ID, fragmentShader); //Прикрепляем фрагментный шейдер

	glLinkProgram(ID); //Связываем шейдеры в единое целое
	compileErrors(ID, "PROGRAM");

	// Удаляем исходные шейдеры (они уже внутри программы)
	glDeleteShader(vertexShader); //Очищаем память
	glDeleteShader(fragmentShader); //Очищаем память
}

// Метод для активации этой шейдерной программы
void Shader::Activate() {
	glUseProgram(ID); // Включаем программу для отрисовки следующих объектов
}

// Метод для очистки памяти
void Shader::Delete() {
	glDeleteProgram(ID); // Удаляем шейдерную программу с видеокарты
}

void Shader::compileErrors(unsigned int shader, const char* type) {
	GLint hasCompiled;
	char infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER COMPILATION ERROR for:" << type << "\n" << std::endl;
		}
	}
	else {
		glGetProgramiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER LINKING ERROR for:" << type << "\n" << std::endl;
		}
	}
}

void Shader::SetVec3(const std::string name, float x, float y, float z) {
	GLint location = glGetUniformLocation(ID, name.c_str());
	if (location == -1) {
		std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
	}
	else {
		glUniform3f(location, x, y, z);
	}
}

void Shader::SetVec2(const std::string name, float x, float y) {
	GLint location = glGetUniformLocation(ID, name.c_str());
	if (location == -1) {
		std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
	}
	else {
		glUniform2f(location, x, y);
	}
}

void Shader::SetFloat(const std::string name, float value) {
	GLint location = glGetUniformLocation(ID, name.c_str());
	if (location == -1) {
		std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
	}
	else {
		glUniform1f(location, value);
	}
}