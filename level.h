#ifndef LEVEL_H
#define LEVEL_H

#define LEVEL_WIDTH 32
#define LEVEL_HEIGHT 32

struct Tile {
	int sprite;
	int solidOT;
	int solidOL;
	int solidOR;
	int solidOB;
	int jumpBoost;
	int harmful;
	int instaDeath;
};

struct Obj {
	int jumpBoostX;
	int jumpBoostY;
};

extern struct Tile level[LEVEL_HEIGHT][LEVEL_WIDTH];

#endif