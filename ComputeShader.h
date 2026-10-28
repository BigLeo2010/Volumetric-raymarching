#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include<iostream>
#include<glad/glad.h>
#include<fstream>
#include<sstream>
#include<cerrno>
#include<vector>
#include<cmath>
#include"shaderClass.h"

class ComputeShader {
public:
	GLuint ID;

	ComputeShader(const char* computePath);

	void Activate();
	void Delete();
private:
	void compileErrors(unsigned int shader, const char* type);
};

#endif
