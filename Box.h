#ifndef BOX_H
#define BOX_H

#include"SimplexNoise.h"

class Box {
public:
	static constexpr int GRID_X = 64;
	static constexpr int GRID_Y = 64;
	float grid[GRID_X][GRID_Y][GRID_X];

	float frequency = 0.1f;
	float amplitude = 1.0f;

	SimplexNoise noise;

	void FillGrid();
};

#endif
