#include "ComputeShader.h"

ComputeShader::ComputeShader(const char* computePath) {
	std::string computeCode = get_file_contents(computePath);

	const char* computeFile = computeCode.c_str();

	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &computeFile, NULL);
	glCompileShader(computeShader);
	compileErrors(computeShader, "COMPUTE");

	ID = glCreateProgram();
	glAttachShader(ID, computeShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	glDeleteShader(computeShader);
}

void ComputeShader::Activate() {
	glUseProgram(ID);
}

void ComputeShader::Delete() {
	glDeleteProgram(ID);
}

void ComputeShader::compileErrors(unsigned int shader, const char* type) {
	GLint success;
	char infoLog[1024];

	if (std::string(type) != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER COMPILATION ERROR for: " << type << "\n" << infoLog << std::endl;
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (success == GL_FALSE) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER LINKING ERROR for: " << type << "\n" << infoLog << std::endl;
		}
	}
}