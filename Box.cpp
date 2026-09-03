#include "Box.h"

void Box::FillGrid() {
	for (int x = 0; x < GRID_X; x++) {
		for (int y = 0; y < GRID_Y; y++) {
			for (int z = 0; z < GRID_X; z++) {

				double rawNoise = noise.noise(x * frequency, y * frequency, z * frequency);

				grid[x][y][z] = (float)rawNoise;
			}
		}
	}
}