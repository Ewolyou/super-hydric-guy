#ifndef LEVEL_H
#define LEVEL_H

#include <raylib.h>

#define LEVEL_WIDTH 32
#define LEVEL_HEIGHT 32
#define LEVEL_WIDTH_PIXEL (LEVEL_WIDTH*16)
#define LEVEL_HEIGHT_PIXEL (LEVEL_HEIGHT*16)

#define BACKGROUND_COLOR_GLOBAL BLACK

#include "AI_list.h"

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

typedef struct {
	SpriteAI AI;
	char AIName[MAX_TEXT_CHAR];
	int textureID;
	float width;
	float height;
	float x;
	float y;
	int unused;
	float texWidth;
	float texHeight;
	char textureName[MAX_TEXT_CHAR];
} FinalSprite;

extern struct Tile level[LEVEL_HEIGHT][LEVEL_WIDTH];
extern FinalSprite sprites[SPRITE_LIMIT];
extern char levelMusicText[MAX_TEXT_CHAR];
extern char levelBGText[MAX_TEXT_CHAR];
extern char levelTilesetText[MAX_TEXT_CHAR];

#endif