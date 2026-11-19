#ifndef AI_LIST_H
#define AI_LIST_H

//#include "level.h"

#define AI_STATE_LIMIT 32
#define AI_LIMIT 32
#define SPRITE_LIMIT 64

#define MAX_TEXT_CHAR 64
#define UNUSED_TEXT "Unused"

typedef struct {
	int chasesX; // chases the player in the X axis
	int chasesY; // chases the player in the Y axis [ USE WITH (int)floats !!! ]
	int movesOneWayX; // <0 = left, 0 = off, >0 = right (will not turn if it collides with a wall)
	int movesOneWayY; // <0 = up, 0 = off, >0 = down (will not turn if it collides with blocks)	[ USE WITH (int)floats !!! ]
	int movesStartingLeft; // <0 = indefinitely, 0 = off, >0 = how many tiles it will move (turns right, once it finishes moving through the tiles, or collides with a wall when noClip is 0)
	int movesStartingRight; // <0 = indefinitely, 0 = off, >0 = how many tiles it will move (turns left, once it finishes moving through the tiles, or collides with a wall when noClip is 0)
	int movesStartingUp; // <0 = indefinitely, 0 = off, >0 = how many tiles it will move (turns down, once it finishes moving through the tiles, or collides with blocks when noClip is 0) [ USE WITH (int)floats !!! ]
	int movesStartingDown; // <0 = indefinitely, 0 = off, >0 = how many tiles it will move (turns up, once it finishes moving through the tiles, or collides with blocks when noClip is 0) [ USE WITH (int)floats !!! ]
	//int movedX; // set to 0 - counter for how many tiles it has moved horizontally
	//int movedY; // set to 0 - counter for how many tiles it has moved vertically
	int directionLR; // set to 0 - handled by updateAI() dev note: (0==LEFT,1==RIGHT)
	int directionUD; // set to 0 - handled by updateAI() dev note: (0==UP,1==DOWN)
	float startX; // set to 0 - handled by updateAI()
	float startY; // set to 0 - handled by updateAI()
	int notFirstCall; // set to 0 - handled by updateAI()
	int wantsToGoLR;
	int wantsToGoUD;
	float chaseOffsetLeft; // px
	float chaseOffsetRight; // px
	float chaseOffsetUp; // px
	float chaseOffsetDown; // px

	char chaseOffsetLeftText[10];
	char chaseOffsetRightText[10];
	char chaseOffsetUpText[10];
	char chaseOffsetDownText[10];

	int turnAroundX;
	int turnAroundY;
} AIState;

typedef struct {
	AIState state; // see (struct)AIState for details
	int vulnerableOT; // 0 = off, 1 = vulnerable to non-player, 2 = vulnerable to player, 3 = vulnerable to non-player AND player
	int vulnerableOL; // 0 = off, 1 = vulnerable to non-player, 2 = vulnerable to player, 3 = vulnerable to non-player AND player
	int vulnerableOR; // 0 = off, 1 = vulnerable to non-player, 2 = vulnerable to player, 3 = vulnerable to non-player AND player
	int vulnerableOB; // 0 = off, 1 = vulnerable to non-player, 2 = vulnerable to player, 3 = vulnerable to non-player AND player
	int instaDeath; // 0 = off, 1 = instadeath to non-player, 2 = instadeath to player, 3 = instadeath to non-player AND player
	float maxSpeedX;
	float minSpeedX;
	//float width; // sprite width and its collision/hit box in pixels
	//float height; // sprite height and its collision/hit box in pixels
	float velocityX;
	float velocityY;
	int health; // unused for now
	//int facingLeft; // where the sprite faces
	float rotating; // <0 = anti-clockwise, 0 = off, >0 = clockwise, value determines speed (rotates n degrees per second)
	int floats;  //  will not be affected by gravity
	int noClip;  //   will not collide with the terrain (use with (int)floats otherwise it will just fall through the level)
	float gravity; // used when (int)floats is 0
	float rotation; // set to 0 ( handled by (float)rotating )
	int stateID; // set to 0 (handled by AI Editor)
	char maxSpeedXText[10];
	char minSpeedXText[10];
	char accelXGainText[10];
	char accelYGainText[10];
	char rotatingText[10];
	char gravityText[10];
	int syncRotationAndDirection;
	int eggPowerup;
	int jumpBoostPowerup;
	float maxSpeedY;
	float minSpeedY;
	float accelXGain;
	float accelYGain;
	char maxSpeedYText[10];
	char minSpeedYText[10];
} SpriteAI;

//extern AIState UnusedAIState;
//extern SpriteAI UnusedAI;

extern AIState AIStates[AI_STATE_LIMIT];
extern SpriteAI AIs[AI_LIMIT];
extern char AIStateNames[AI_STATE_LIMIT][64];
extern char AINames[AI_LIMIT][64];

#endif