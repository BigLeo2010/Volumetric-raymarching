#include "NormalGeneration.h"

std::vector<ColorRGB> NormalGeneration::GenerateNormalMap(int targetSize, const std::vector<uint8_t>& cubeBuffer) {
    std::vector<ColorRGB> normalBuffer(targetSize * targetSize * targetSize);

#pragma omp parallel for collapse(2) schedule(static)
    for (int z = 0; z < targetSize; ++z) {
        for (int y = 0; y < targetSize; ++y) {

            const int z_stride = z * targetSize * targetSize;
            const int zm1_stride = std::max(0, z - 1) * targetSize * targetSize;
            const int zp1_stride = std::min(targetSize - 1, z + 1) * targetSize * targetSize;

            const int y_stride = y * targetSize;
            const int ym1_stride = std::max(0, y - 1) * targetSize;
            const int yp1_stride = std::min(targetSize - 1, y + 1) * targetSize;

#pragma omp simd
            for (int x = 0; x < targetSize; ++x) {
                const int xm1 = std::max(0, x - 1);
                const int xp1 = std::min(targetSize - 1, x + 1);

                float dx = static_cast<float>(cubeBuffer[xp1 + y_stride + z_stride]) -
                    static_cast<float>(cubeBuffer[xm1 + y_stride + z_stride]);

                float dy = static_cast<float>(cubeBuffer[x + yp1_stride + z_stride]) -
                    static_cast<float>(cubeBuffer[x + ym1_stride + z_stride]);

                float dz = static_cast<float>(cubeBuffer[x + y_stride + zp1_stride]) -
                    static_cast<float>(cubeBuffer[x + y_stride + zm1_stride]);

                dx /= 255.0f;
                dy /= 255.0f;
                dz /= 255.0f;

                float lengthSq = dx * dx + dy * dy + dz * dz;
                float len = (std::sqrt(lengthSq) + 1e-6f);

                float nx = -dx / len;
                float ny = -dy / len;
                float nz = -dz / len;

                int idx = x + y_stride + z_stride;
                normalBuffer[idx].r = static_cast<uint8_t>((nx * 0.5f + 0.5f) * 255.0f);
                normalBuffer[idx].g = static_cast<uint8_t>((ny * 0.5f + 0.5f) * 255.0f);
                normalBuffer[idx].b = static_cast<uint8_t>((nz * 0.5f + 0.5f) * 255.0f);
            }
        }
    }

    return normalBuffer;
}
