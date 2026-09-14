#ifndef NORMAL_GENERATION_H
#define NORMAL_GENERATION_H

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<string>
#include<fstream>
#include<sstream>
#include<cerrno>
#include<vector>
#include <cmath>

struct ColorRGB {
	uint8_t r, g, b;
};

class NormalGeneration{
public:
	static std::vector<ColorRGB> GenerateNormalMap(int targetSize, std::vector<uint8_t>& cubeBuffer);
};

#endif
