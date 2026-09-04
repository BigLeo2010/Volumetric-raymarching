#include "Box.h"

void Box::FillGrid() {
	for (int x = 0; x < GRID_X; x++) {
		for (int y = 0; y < GRID_Y; y++) {
			for (int z = 0; z < GRID_X; z++) {

				double rawNoise = noise.noise(x * frequency, y * frequency, z * frequency);

				float noiseValue = (float)(rawNoise) * amplitude;

				grid[x][y][z] = noiseValue;
			}
		}
	}
}