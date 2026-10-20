#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include<iostream>
#include<glad/glad.h>
#include<fstream>
#include<sstream>
#include<cerrno>
#include<vector>
#include<cmath>

class ComputeShader {
public:
	unsigned int ID;

	ComputeShader(const char* computePath);
};

#endif
