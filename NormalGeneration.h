#ifndef NORMAL_GENERATION_H
#define NORMAL_GENERATION_H

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

struct ColorRGB {
	uint8_t r, g, b;
};

class NormalGeneration {
public:
	static std::vector<ColorRGB> GenerateNormalMap(int targetSize, std::vector<uint8_t>& cubeBuffer);;
};

#endif
