#include "NormalGeneration.h"

std::vector<ColorRGB> NormalGeneration::GenerateNormalMap(int targetSize, std::vector<uint8_t>& cubeBuffer) {
	std::vector<ColorRGB> normalBuffer(targetSize * targetSize * targetSize);

	auto getDensity = [&](int x, int y, int z) {
		x = std::max(0, std::min(x, targetSize - 1));
		y = std::max(0, std::min(y, targetSize - 1));
		z = std::max(0, std::min(z, targetSize - 1));
		return static_cast<float>(cubeBuffer[x + y * targetSize + z * targetSize * targetSize]) / 255.0f;
	};

	for (int z = 0; z < targetSize; ++z) {
		for (int y = 0; y < targetSize; ++y) {
			for (int x = 0; x < targetSize; ++x) {
				float dx = getDensity(x + 1, y, z) - getDensity(x - 1, y, z);
				float dy = getDensity(x, y + 1, z) - getDensity(x, y - 1, z);
				float dz = getDensity(x, y, z + 1) - getDensity(x, y, z - 1);

				float nx = -dx;
				float ny = -dy;
				float nz = -dz;

				float length = std::sqrt(nx * nx + ny * ny + nz * nz) + 1e-6f;
				nx /= length;
				ny /= length;
				nz /= length;

				int idx = x + y * targetSize + z * targetSize * targetSize;
				normalBuffer[idx].r = static_cast<uint8_t>((nx * 0.5f + 0.5f) * 255.0f);
				normalBuffer[idx].g = static_cast<uint8_t>((ny * 0.5f + 0.5f) * 255.0f);
				normalBuffer[idx].b = static_cast<uint8_t>((nz * 0.5f + 0.5f) * 255.0f);
			}
		}
	}

	return normalBuffer;
}