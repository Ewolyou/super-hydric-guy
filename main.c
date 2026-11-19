#include <raylib.h>
#include <stdio.h>
#include "level.h"
#include "AI_list.h"

#define EGG_POWERUP_WIDTH 16
#define EGG_POWERUP_HEIGHT 16
#define JB_POWERUP_WIDTH 16
#define JB_POWERUP_HEIGHT 16

#define EGG_POWERUP 1
#define JB_POWERUP 2

#define SMALL NORMAL
#define NORMAL 0
#define SUPER 1
#define JUMP_BOOST 2
#define MINI 3
#define POWERUP_MAX 3
#define MAX_POWERUP_SPAWN 5

#define DMG_INVINC_TIME 2
#define GROUND_POUND_STALL_TIME 0.25f
#define TWIRL_COOLDOWN 0.53f
#define PLAYER_JUMPFORCE -350.0f
#define PLAYER_GRAVITY 800.0f
#define SPRITE_GRAVITY 799.99f // !!!!! ALL JUST FORMER :  "199.99f (formerly 207.33f idk why this happened) is the "max" before collision breaks for some reason, might fix later"

#define PLAYER_WIDTH_BASE 13

#define SUBJECT_BOTTOM 0
#define SUBJECT_TOP 1
#define SUBJECT_LEFT 2
#define SUBJECT_RIGHT 3

#define LEFT 0
#define RIGHT 1
#define UP 0
#define DOWN 1

#define X 1
#define Y 0

struct Sounds {
	Sound jumpSound;
	Sound doubleJumpSound;
	Sound playerHurtSound;
	Sound powerUpSound;
	Sound blockPowerUpThrowSound;
	Sound playerDeadSound;
	Sound twirlSound;
	Sound miniTwirlSound;
	Sound miniJumpSound;
	Sound miniFrictionSlideSound;
};

struct Player {
	int grounded;
	int groundPounding;
	int doubleJump;
	int powerup;
	float invincible;
	int dead;
	float gravity;
	float x;
	float y;
	int crouching;
	int frictionSliding;
	int frictionSlideLeft;
	int slowingDown;
	int slowDownLeft;
	int twirling;
};

float maxSpeed = 180.0f;
float minSpeed = 80.0f;

int advanceEggPowerup = 0;
int advanceJBPowerup = 0;

int usedEggPowerup[MAX_POWERUP_SPAWN] = {0};
int freeEggPowerupSlot = 0;
Vector2 eggPowerupPos[MAX_POWERUP_SPAWN] = {0};

int usedJBPowerup[MAX_POWERUP_SPAWN] = {0};
int freeJBPowerupSlot = 0;
Vector2 JBPowerupPos[MAX_POWERUP_SPAWN] = {0};


void switchPowerup(struct Player *player, int downgrade, int targetPowerup, struct Sounds sounds) {
	if(downgrade) {
		if(player->powerup == SMALL || player->powerup == MINI) player->dead = 1;
		else if(player->powerup == SUPER) {
			player->y += 14;
			player->powerup = SMALL;
		}
		else if(player->powerup == JUMP_BOOST) player->powerup = SUPER;
		if(!player->dead) {
			PlaySound(sounds.playerHurtSound);
			player->invincible = DMG_INVINC_TIME;
			player->doubleJump = 0;
		}
	}
	else {
		PlaySound(sounds.powerUpSound);
		if(targetPowerup < 0) targetPowerup = 0;
		else if(targetPowerup > POWERUP_MAX) targetPowerup = POWERUP_MAX;
		if(player->powerup == SMALL && (targetPowerup == SUPER || targetPowerup == JUMP_BOOST))
			player->y -= 16;
		player->powerup = targetPowerup;
		if(targetPowerup == JUMP_BOOST) player->doubleJump = 1;
	}
}

int spawnPowerup(int powerup, int xTile, int yTile) {
	float xPixel = xTile*16;
	float yPixel = yTile*16;

	if(powerup == EGG_POWERUP && freeEggPowerupSlot < MAX_POWERUP_SPAWN) {
		usedEggPowerup[freeEggPowerupSlot] = 1;
		eggPowerupPos[freeEggPowerupSlot].x = xPixel;
		eggPowerupPos[freeEggPowerupSlot].y = yPixel;
		advanceEggPowerup++;
		return 1;
	}
	else if(powerup == JB_POWERUP && freeJBPowerupSlot < MAX_POWERUP_SPAWN) {
		usedJBPowerup[freeJBPowerupSlot] = 1;
		JBPowerupPos[freeJBPowerupSlot].x = xPixel;
		JBPowerupPos[freeJBPowerupSlot].y = yPixel;
		advanceJBPowerup++;
		return 1;
	}

	return 0;
}

int despawnPowerup(int powerup, int slot) {
	if(slot < MAX_POWERUP_SPAWN) {
		if(powerup == EGG_POWERUP) {
			if(usedEggPowerup[slot]) {
				usedEggPowerup[slot] = 0;
				return 1;
			}
		}
		else if(powerup == JB_POWERUP) {
			if(usedJBPowerup[slot]) {
				usedJBPowerup[slot] = 0;
				return 1;
			}
		}
	}

	return 0;
}

int checkCollisionOfSubject(int side, float *subjectX, float *subjectY, float subjectWidth, float subjectHeight, float *velocityY, float *velocityX, struct Player *player, struct Sounds sounds, struct Obj Object[2], int isPlayer, SpriteAI *AI)
{
	int leftTile = (int)*subjectX / 16;
	int rightTile = (int)(*subjectX + subjectWidth-1) / 16;

	int bottomTile = (int)(*subjectY + subjectHeight-1) / 16;
	int topTile = (int)*subjectY / 16;

	int bottomPixel = (int)*subjectY + subjectHeight;
	int bottomTileForBottom = bottomPixel / 16;

	Rectangle subjectRec = {*subjectX, *subjectY, subjectWidth, subjectHeight}


	int X1Tile = 0;
	int X2Tile = 0;
	int Y1Tile = 0;
	int Y2Tile = 0;

	int rcode = 0;

	if(side == SUBJECT_BOTTOM) {
		X1Tile = leftTile;
		X2Tile = rightTile;
		Y1Tile = bottomTileForBottom;
		Y2Tile = bottomTileForBottom;
	}
	else if(side == SUBJECT_TOP) {
		X1Tile = leftTile;
		X2Tile = rightTile;
		Y1Tile = topTile;
		Y2Tile = topTile;
	}
	else if(side == SUBJECT_LEFT) {
		if(*subjectX < 0) {
			*subjectX = 0;
			return 1;}
		X1Tile = leftTile;
		X2Tile = leftTile;
		Y1Tile = topTile;
		Y2Tile = bottomTile;
	}
	else if(side == SUBJECT_RIGHT) {
		if(*subjectX + subjectWidth > LEVEL_WIDTH * 16) {
			*subjectX = (LEVEL_WIDTH*16) - subjectWidth;
			return 1;}
		X1Tile = rightTile;
		X2Tile = rightTile;
		Y1Tile = topTile;
		Y2Tile = bottomTile;
	}
	
	float X1Pixel = X1Tile*16;
	float X2Pixel = X2Tile*16;
	float Y1Pixel = Y1Tile*16;
	float Y2Pixel = Y2Tile*16;

	if(X1Tile >= 0 && X1Tile < LEVEL_WIDTH &&
	X2Tile >= 0 && X2Tile < LEVEL_WIDTH &&
	Y1Tile >= 0 && Y1Tile < LEVEL_HEIGHT &&
	Y2Tile >= 0 && Y2Tile < LEVEL_HEIGHT)
	{
		if(side == SUBJECT_BOTTOM){
			int solid = 0;
			if(level[Y1Tile][X1Tile].solidOT ||
			level[Y2Tile][X2Tile].solidOT)
					solid = 1;
			if(isPlayer && !(*velocityY >= 0)) return 0;

			if(isPlayer && player->groundPounding && (level[Y2Tile][X1Tile].jumpBoost || level[Y2Tile][X2Tile].jumpBoost)) {	
				if(level[Y2Tile][X1Tile].jumpBoost) {
					PlaySound(sounds.blockPowerUpThrowSound);

					if(player->powerup == MINI || player->powerup == SMALL)
						spawnPowerup(EGG_POWERUP, X1Tile, Y2Tile+1);
					else {
						spawnPowerup(JB_POWERUP, X1Tile, Y2Tile+1);
					}

					level[Y2Tile][X1Tile].jumpBoost = 0;
					level[Y2Tile][X1Tile].sprite = 29;
				}
				if(level[Y2Tile][X2Tile].jumpBoost) {
					PlaySound(sounds.blockPowerUpThrowSound);

					if(player->powerup == MINI || player->powerup == SMALL)
						spawnPowerup(EGG_POWERUP, X2Tile, Y2Tile+1);
					else {
						spawnPowerup(JB_POWERUP, X2Tile, Y2Tile+1);
					}

					level[Y2Tile][X2Tile].jumpBoost = 0;
					level[Y2Tile][X2Tile].sprite = 29;
				}
			}
			if(solid) {
				*subjectY = Y2Tile * 16 - subjectHeight;
				rcode = 1;
				
				if(isPlayer) {
					player->grounded = 1;
					player->groundPounding = 0;
					*velocityY = 0;
					if(player->powerup == JUMP_BOOST) player->doubleJump = 1;
					
				}
				else if(!AI->floats) *velocityY = 0;
			}
		}
		if(side == SUBJECT_TOP) {
			int solid = 0;
			if(level[Y1Tile][X1Tile].solidOB ||
			level[Y2Tile][X2Tile].solidOB)
				solid = 1;
			if(isPlayer && !(*velocityY < 0)) return 0;
			if(solid) {
				*subjectY = Y1Tile * 16 + 16;
				*velocityY = 50;
				rcode = 1;
			}

			if(isPlayer) {
				if(level[Y1Tile][X1Tile].jumpBoost || level[Y1Tile][X2Tile].jumpBoost) {
					if(level[Y1Tile][X1Tile].jumpBoost) {
						PlaySound(sounds.blockPowerUpThrowSound);

						if(player->powerup == MINI || player->powerup == SMALL)
							spawnPowerup(EGG_POWERUP, X1Tile, Y1Tile-1);
						else {
							spawnPowerup(JB_POWERUP, X1Tile, Y1Tile-1);
						}

						level[Y1Tile][X1Tile].jumpBoost = 0;
						level[Y1Tile][X1Tile].sprite = 29;
					}
					if(level[Y1Tile][X2Tile].jumpBoost) {
						PlaySound(sounds.blockPowerUpThrowSound);

						if(player->powerup == MINI || player->powerup == SMALL)
							spawnPowerup(EGG_POWERUP, X2Tile, Y1Tile-1);
						else {
							spawnPowerup(JB_POWERUP, X2Tile, Y1Tile-1);
						}

						level[Y1Tile][X2Tile].jumpBoost = 0;
						level[Y1Tile][X2Tile].sprite = 29;
					}
				}
			}
		}
		if(side == SUBJECT_LEFT) {
			if(level[Y1Tile][X1Tile].solidOR ||
			level[Y2Tile][X2Tile].solidOR) {
				*subjectX = (X1Tile*16) + 16;
				rcode = 1;
			}
		}
		if(side == SUBJECT_RIGHT){
			if(level[Y1Tile][X1Tile].solidOL ||
			level[Y2Tile][X2Tile].solidOL) {
				*subjectX = (X2Tile*16) - subjectWidth;
				rcode = 1;
			}
		}

		if(isPlayer) {

			for(int i = 0; i < MAX_POWERUP_SPAWN; i++) {
				if(usedEggPowerup[i]) {
					if( (eggPowerupPos[i].x == X1Pixel || eggPowerupPos[i].x == X2Pixel) &&
						(eggPowerupPos[i].y == Y1Pixel || eggPowerupPos[i].y == Y2Pixel) ) {
							switchPowerup(player, 0, SUPER, sounds);
							despawnPowerup(EGG_POWERUP, i);
					}
				}

				if(usedJBPowerup[i]) {
					if( (JBPowerupPos[i].x == X1Pixel || JBPowerupPos[i].x == X2Pixel) &&
						(JBPowerupPos[i].y == Y1Pixel || JBPowerupPos[i].y == Y2Pixel) ) {
							switchPowerup(player, 0, JUMP_BOOST, sounds);
							despawnPowerup(JB_POWERUP, i);
					}
				}
			}

			if(level[Y1Tile][X1Tile].harmful ||
			level[Y2Tile][X2Tile].harmful) {
				if(!player->invincible) {
					switchPowerup(player, 1, 0, sounds);
				}
			}

			if(level[Y1Tile][X1Tile].instaDeath ||
			level[Y2Tile][X2Tile].instaDeath)
				player->dead = 1;
		}
	}
	else if(X1Tile < 0 || X1Tile > LEVEL_WIDTH ||
			X2Tile < 0 || X2Tile > LEVEL_WIDTH )
				rcode = 1;

	if(isPlayer && rcode == 1 && (side == SUBJECT_LEFT || side == SUBJECT_RIGHT)) {
		*velocityX = minSpeed;
	}
	
	return rcode;
}

void moveAI(int x, SpriteAI *AI, float *posX, float *posY, float width, float dt) {
	if(x) {
		// GO LEFT
		/*if(AI->state.wantsToGoLR == LEFT) {
			if(AI->state.directionLR == RIGHT) AI->velocityX = AI->minSpeedX;
			AI->velocityX += AI->accelXGain;
			*posX -= AI->velocityX * dt;
			AI->state.directionLR = LEFT;

			//if(*posX < 0) *posX = 0;
		}

		// GO RIGHT
		if(AI->state.wantsToGoLR == RIGHT) {
			if(AI->state.directionLR == LEFT) AI->velocityX = AI->minSpeedX;
			AI->velocityX += AI->accelXGain;
			*posX += AI->velocityX * dt;
			AI->state.directionLR = RIGHT;

			/*if(*posX > LEVEL_WIDTH_PIXEL - width)
				*posX = LEVEL_WIDTH_PIXEL - width;*/

		if(AI->state.turnAroundX) {
			AI->velocityX = AI->minSpeedX;
			AI->state.turnAroundX = 0;
		}
		else AI->velocityX += AI->accelXGain;

		if(AI->velocityX > AI->maxSpeedX) AI->velocityX = AI->maxSpeedX;
		if(AI->velocityX < AI->minSpeedX) AI->velocityX = AI->minSpeedX;

		if(AI->state.directionLR == LEFT)
			*posX -= AI->velocityX * dt;
		else
			*posX += AI->velocityX * dt;
	}
	else {
		if(AI->state.turnAroundY) {
			AI->velocityY = AI->minSpeedY;
			AI->state.turnAroundY = 0;
		}
		else AI->velocityY += AI->accelYGain;

		if(AI->velocityY > AI->maxSpeedY) AI->velocityY = AI->maxSpeedY;
		if(AI->velocityY < AI->minSpeedY) AI->velocityY = AI->minSpeedY;

		if(AI->state.directionUD == UP)
			*posY -= AI->velocityY * dt;
		else
			*posY += AI->velocityY * dt;
	}
}

void updateAI(SpriteAI *AI, Texture2D sprite, float *posX, float *posY, float width, float height, int spriteFrame, float playerX, float playerY, float dt) {
	// very complicated thing (syncs rotating direction with horizontal movement direction)
	float rotating = AI->rotating;
	if(AI->state.movesStartingLeft || AI->state.movesOneWayX < 0 || (AI->state.chasesX && playerX < *posX))
		rotating = AI->syncRotationAndDirection ? (AI->state.directionLR == RIGHT ? (-rotating) : rotating) : rotating;
	else rotating = AI->syncRotationAndDirection ? (AI->state.directionLR == LEFT ? (-rotating) : rotating) : rotating;
	
	AI->rotation += rotating * dt;
	if(AI->rotation >= 360 || AI->rotation <= -360) AI->rotation = 0;

	struct Player miscSprite = {0, 0, 0, 0, 0, 0, AI->gravity};
	struct Obj objSprite[2] = {0};
	struct Sounds soundsSprite = {
		0, //LoadSound("audio/sfx/jump.wav"),
		0, //LoadSound("audio/sfx/double_jump.wav"),
		0, //LoadSound("audio/sfx/player_hurt.wav"),
		0, //LoadSound("audio/sfx/powerup.wav"),
		0, //LoadSound("audio/sfx/block_powerup_throw.wav"),
		0 //LoadSound("audio/sfx/player_dead.wav")
	};

	if(!AI->state.notFirstCall) { // FIRST CALL ONLY ( STARTUP CONFIG )
		AI->state.startX = *posX;
		AI->state.startY = *posY;

		if(AI->state.movesStartingLeft || AI->state.movesOneWayX < 0) {
			AI->state.directionLR = LEFT;
			AI->state.wantsToGoLR = LEFT;
		}
		else if(AI->state.movesStartingRight || AI->state.movesOneWayX > 0) {
			AI->state.directionLR = RIGHT;
			AI->state.wantsToGoLR = RIGHT;
		}

		if(AI->state.movesStartingUp || AI->state.movesOneWayY < 0) {
			AI->state.directionUD = UP;
			AI->state.wantsToGoUD = UP;
		}
		else if(AI->state.movesStartingDown || AI->state.movesOneWayY > 0) {
			AI->state.directionUD = DOWN;
			AI->state.wantsToGoUD = DOWN;
		}

		if(!AI->floats) AI->velocityY = 0;

		AI->state.notFirstCall = 1;
	}

	if(AI->state.movesOneWayX) {
		if(AI->state.movesOneWayX < 0) {AI->state.wantsToGoLR = LEFT; }//*posX -= AI->velocityX * dt;
		else if(AI->state.movesOneWayX > 0) {AI->state.wantsToGoLR = RIGHT;}//*posX += AI->velocityX * dt;
		moveAI(X, AI, posX, posY, width, dt);
	}
	if(AI->state.movesOneWayY) {
		if(AI->state.movesOneWayY < 0) {AI->state.wantsToGoUD = UP;}//*posY -= AI->velocityY * dt;
		else if(AI->state.movesOneWayY > 0) {AI->state.wantsToGoUD = DOWN;}//*posY += AI->velocityY * dt;
		moveAI(Y, AI, posX, posY, width, dt);
	}

	if(AI->state.movesStartingLeft || AI->state.movesStartingRight) {
		if(AI->state.movesStartingLeft < 0 || AI->state.movesStartingRight < 0) {
			if(AI->state.directionLR == LEFT) {
				AI->state.wantsToGoLR = LEFT;//*posX -= AI->velocityX * dt;
				AI->state.directionLR = LEFT;
			}
			else {
				AI->state.wantsToGoLR = RIGHT;//*posX += AI->velocityX * dt;
				AI->state.directionLR = RIGHT;
			}

			moveAI(X, AI, posX, posY, width, dt);
		}
		else {
			float movesStartingDirection = AI->state.movesStartingLeft ? AI->state.movesStartingLeft : AI->state.movesStartingRight;
			
			float distance = movesStartingDirection * 16.0f;
			float targetX = AI->state.directionLR == RIGHT ? AI->state.startX + distance : AI->state.startX - distance;

			if(AI->state.directionLR == RIGHT) {
				//*posX += AI->velocityX * dt;
				AI->state.wantsToGoLR = RIGHT;
				moveAI(X, AI, posX, posY, width, dt);

				if(*posX >= targetX) {
					*posX = targetX;
					AI->state.startX = *posX;
					AI->state.directionLR = LEFT;
					AI->state.wantsToGoLR = LEFT;
				}
			}
			else {
				//*posX -= AI->velocityX * dt;
				AI->state.wantsToGoLR = LEFT;
				moveAI(X, AI, posX, posY, width, dt);

				if(*posX <= targetX) {
					*posX = targetX;
					AI->state.startX = *posX;
					AI->state.directionLR = RIGHT;
					AI->state.wantsToGoLR = RIGHT;
				}
			}
		}
	}

	if(AI->state.movesStartingUp || AI->state.movesStartingDown) {
		if(AI->state.movesStartingUp < 0 || AI->state.movesStartingDown < 0) {
			if(AI->state.directionUD == UP) {
				AI->state.directionUD = UP;
				AI->state.wantsToGoUD = UP;
			}
			else {
				AI->state.directionUD = DOWN;
				AI->state.wantsToGoUD = DOWN;
			}

			moveAI(Y, AI, posX, posY, width, dt);
		}
		else {
			float movesStartingDirection = AI->state.movesStartingUp ? AI->state.movesStartingUp : AI->state.movesStartingDown;
			
			float distance = movesStartingDirection * 16.0f;
			float targetY = AI->state.directionUD == DOWN ? AI->state.startY + distance : AI->state.startY - distance;

			if(AI->state.directionUD == DOWN) {
				//*posY += AI->velocityY * dt;
				AI->state.wantsToGoUD = DOWN;
				moveAI(Y, AI, posX, posY, width, dt);
				if(*posY >= targetY) {
					*posY = targetY;
					AI->state.startY = *posY;
					AI->state.directionUD = UP;
					AI->state.wantsToGoUD = UP;
				}
			}
			else {
				//*posY -= AI->velocityY * dt;
				AI->state.wantsToGoUD = UP;
				moveAI(Y, AI, posX, posY, width, dt);
				if(*posY <= targetY) {
					*posY = targetY;
					AI->state.startY = *posY;
					AI->state.directionUD = DOWN;
					AI->state.wantsToGoUD = DOWN;
				}
			}
		}
	}

	if(AI->state.chasesX) {
		if(playerX+AI->state.chaseOffsetRight < *posX) {//						LEFT
			//*posX -= AI->velocityX * dt;
			AI->state.directionLR = LEFT;
			AI->state.wantsToGoLR = LEFT;
		}
		else if(playerX-AI->state.chaseOffsetLeft > *posX) {//					RIGHT
			//*posX += AI->velocityX * dt;
			AI->state.directionLR = RIGHT;
			AI->state.wantsToGoLR = RIGHT;
		}
		moveAI(X, AI, posX, posY, width, dt);
	}

	if(AI->state.chasesY) {
		if(playerY+AI->state.chaseOffsetDown < *posY) {//						UP
			//*posY -= AI->velocityY * dt;
			AI->state.directionUD = UP;
			AI->state.wantsToGoUD = UP;
		}
		else if(playerY-AI->state.chaseOffsetUp > *posY) {//					DOWN
			//*posY += AI->velocityY * dt;
			AI->state.directionUD = DOWN;
			AI->state.wantsToGoUD = DOWN;
		}
		moveAI(Y, AI, posX, posY, width, dt);
	}

	if(!AI->floats) {
		AI->velocityY += miscSprite.gravity * dt;
		*posY += AI->velocityY * dt;
	}

	if(!AI->noClip) {
		//if(AI->velocityX > 0 && AI->state.directionLR == 0) {
			if(checkCollisionOfSubject(SUBJECT_LEFT, posX, posY, width, height, &AI->velocityY, &AI->velocityX, &miscSprite, soundsSprite, objSprite, 0, AI))
				if(AI->state.movesStartingLeft || AI->state.movesStartingRight) {
					//if(AI->state.directionLR == LEFT) AI->state.wantsToGoLR = RIGHT;
					AI->state.directionLR = RIGHT;
					AI->state.wantsToGoLR = RIGHT;
					AI->state.startX = *posX;
					AI->state.turnAroundX = 1;
				}
		//}
		//else if(AI->velocityX > 0 && AI->state.directionLR == 1) {
			if(checkCollisionOfSubject(SUBJECT_RIGHT, posX, posY, width, height, &AI->velocityY, &AI->velocityX, &miscSprite, soundsSprite, objSprite, 0, AI))
				if(AI->state.movesStartingLeft || AI->state.movesStartingRight) {
					//if(AI->state.directionLR == RIGHT) AI->state.wantsToGoLR = LEFT;
					AI->state.directionLR = LEFT;
					AI->state.wantsToGoLR = LEFT;
					AI->state.startX = *posX;
					AI->state.turnAroundX = 1;
				}
		//}

		if(checkCollisionOfSubject(SUBJECT_TOP, posX, posY, width, height, &AI->velocityY, &AI->velocityX, &miscSprite, soundsSprite, objSprite, 0, AI))
			if(AI->state.movesStartingUp || AI->state.movesStartingDown) {
				AI->state.directionUD = DOWN;
				AI->state.wantsToGoUD = DOWN;
				AI->state.startY = *posY;
				AI->state.turnAroundY = 1;
			}
		
		if(checkCollisionOfSubject(SUBJECT_BOTTOM, posX, posY, width, height, &AI->velocityY, &AI->velocityX, &miscSprite, soundsSprite, objSprite, 0, AI))
			if(AI->state.movesStartingUp || AI->state.movesStartingDown) {
				AI->state.directionUD = UP;
				AI->state.wantsToGoUD = UP;
				AI->state.startY = *posY;
				AI->state.turnAroundY = 1;
			}
	}
	float sourceWidth = AI->state.directionLR ? -width : width;

	//   				 ============ DEBUG ============
	//DrawText(TextFormat("directionLR: %d", AI->state.directionLR), 10, 10, 20, RED);
	
	DrawTexturePro(sprite, (Rectangle){spriteFrame*width, 0, sourceWidth, height},
						   (Rectangle){(*posX)+8, (*posY)+8, width, height},
						   (Vector2){width/2.0f, height/2.0f}, AI->rotation, WHITE);
}

int main(void)
{
	InitWindow(LEVEL_WIDTH_PIXEL, LEVEL_HEIGHT_PIXEL, "Super Hydric Guy");
	InitAudioDevice();

	SetTargetFPS(60);

	char levelBGTextModif[MAX_TEXT_CHAR+20] = {0};
	snprintf(levelBGTextModif, sizeof(levelBGTextModif), "gfx/bg/%s", levelBGText);
	char levelTilesetTextModif[MAX_TEXT_CHAR+20] = {0};
	snprintf(levelTilesetTextModif, sizeof(levelTilesetTextModif), "gfx/tilesets/%s", levelTilesetText);

	AIState eggPowerupAIState = {.movesStartingRight = -1, 0};
	SpriteAI eggPowerupAI[MAX_POWERUP_SPAWN] = {0};

	AIState JBPowerupAIState = {0};
	SpriteAI JBPowerupAI[MAX_POWERUP_SPAWN] = {0};

	for(int i = 0; i < MAX_POWERUP_SPAWN; i++) {
		eggPowerupAI[i].state = eggPowerupAIState;
		eggPowerupAI[i].gravity = SPRITE_GRAVITY;
		eggPowerupAI[i].rotating = 200.0f;
		eggPowerupAI[i].maxSpeedX = 50.0f;
		eggPowerupAI[i].minSpeedX = 50.0f;
		eggPowerupAI[i].accelXGain = 50.0f;
		eggPowerupAI[i].syncRotationAndDirection = 1;

		JBPowerupAI[i].state = JBPowerupAIState;
		JBPowerupAI[i].gravity = SPRITE_GRAVITY;
	}

	Texture2D bg = LoadTexture(levelBGTextModif);
	Texture2D playerTX = LoadTexture("gfx/player/player.png");
	Texture2D playerJB = LoadTexture("gfx/player/playerJB.png");
	Texture2D tileset1 = LoadTexture(levelTilesetTextModif);
	Texture2D objects = {0}; //LoadTexture("gfx/objects.png");
	Texture2D hurtle = LoadTexture("gfx/sprites/hurtle.png");

	Texture2D eggPowerupTX = LoadTexture("gfx/sprites/eggPowerup.png");
	Texture2D JBPowerupTX = LoadTexture("gfx/sprites/jumpBoostPowerup.png");

	FilePathList files = LoadDirectoryFiles("gfx/sprites");
	Texture2D *textures = MemAlloc(files.count * sizeof(Texture2D));
	int textureCount = 0;

	for(unsigned int i = 0; i < files.count; i++) {
		if(IsFileExtension(files.paths[i], ".png") ||
			IsFileExtension(files.paths[i], ".jpg")) {
				textures[textureCount++] = LoadTexture(files.paths[i]);
		}
	}
	UnloadDirectoryFiles(files);

	struct Sounds sounds = {
		LoadSound("audio/sfx/jump.wav"),
		LoadSound("audio/sfx/double_jump.wav"),
		LoadSound("audio/sfx/player_hurt.wav"),
		LoadSound("audio/sfx/powerup.wav"),
		LoadSound("audio/sfx/block_powerup_throw.wav"),
		LoadSound("audio/sfx/player_dead.wav"),
		LoadSound("audio/sfx/twirl.wav"),
		LoadSound("audio/sfx/mini_twirl.wav"),
		LoadSound("audio/sfx/mini_jump.wav"),
		LoadSound("audio/sfx/mini_friction_slide.wav")
	};

	SetSoundVolume(sounds.jumpSound, 0.5f);
	SetSoundVolume(sounds.miniJumpSound, 0.5f);
	SetSoundVolume(sounds.miniFrictionSlideSound, 0.5f);

	char levelMusicTextModif[MAX_TEXT_CHAR+20] = {0};
	snprintf(levelMusicTextModif, sizeof(levelMusicTextModif), "audio/music/%s", levelMusicText);

	Music levelMusic = LoadMusicStream(levelMusicTextModif);

	PlayMusicStream(levelMusic);

	//						 CX CY OX OY ML MR MU MD M  M  M  M  M
	AIState __testAIState = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//						AI State	HT HL HR HB iD MX MN VX  VY  HTH R  F   no clip	          GR        M
	SpriteAI __testAI = {__testAIState, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 0, 1, .noClip = 0, SPRITE_GRAVITY, 0};

	int frame = 0;
	float twirlFrame = 16.0f;
	float timer = 0.0f;
	
	struct Player player = {
		.grounded = 0,
		.groundPounding = 0,
		.doubleJump = 0,
		.powerup = SUPER,
		.invincible = 0.0f,
		.dead = 0,
		.gravity = PLAYER_GRAVITY,
		.x = 0.0f,
		.y = 0.0f,
		.crouching = 0,
		.slowingDown = 0,
		.slowDownLeft = 0,
		.twirling = 0,
		.frictionSliding = 0,
		.frictionSlideLeft = 0
	};

	for(int x = 0; x < LEVEL_WIDTH; x++) {
		for(int y = 0; y < LEVEL_HEIGHT; y++) {
			if(level[y][x].sprite == 255) {
				player.x = x*16;
				player.y = (y-4)*16;
			}
		}
	}

	float velocityX = 0.0f;
	float velocityY = 0.0f;

	float jumpForce = PLAYER_JUMPFORCE;
	float groundPoundForce = 600.0f;

	float jumpBuffer = 0.0f;
	float twirlTimer = 0.0f;
	float twirlCooldown = 0.0f;
	float groundPoundBuffer = 0.0f;
	float groundPoundTimer = 0.0f;
	int facingLeft = 0;

	struct Obj Object[2];

	for(int i = 0; i < 2; i++) {
		Object[i].jumpBoostX = -1;
		Object[i].jumpBoostY = -1;
	}

	int invinc_frame = 0;

	int playerDeadSoundPlayed = 0;

	float frictionSlideAccelXLoss = 50.0f;
	float accelXGain = 1.5f;
	float accelXLoss = 2.0f;

	float playerWidth = PLAYER_WIDTH_BASE;
	float playerWidthTexture = 16;
	float playerHeight = 32;
	float playerRotation = 0.0f;

	//float hurtleX = 26*16; float hurtleY = 12*16;
	//float hurtleW = 16; float hurtleH = 16;

	for(int i = 0; i < SPRITE_LIMIT; i++) {
		if(!sprites[i].unused) {
			sprites[i].AI = AIs[sprites[i].AI_ID];
		}
	}

	while(!WindowShouldClose())
	{
		UpdateMusicStream(levelMusic);	
		
		float dt = GetFrameTime();
		//player.powerup = MINI;
		if(player.powerup == NORMAL) {
			player.gravity = PLAYER_GRAVITY;
			jumpForce = PLAYER_JUMPFORCE;
			playerWidth = PLAYER_WIDTH_BASE; playerHeight = 16;}
		else if(player.powerup == SUPER ||
		        player.powerup == JUMP_BOOST) {
					player.gravity = PLAYER_GRAVITY;
					jumpForce = PLAYER_JUMPFORCE;
					playerWidth = PLAYER_WIDTH_BASE; playerHeight = 32;}
		if(player.powerup == MINI) {
			player.gravity = PLAYER_GRAVITY - 480.0f; // 410
			jumpForce = PLAYER_JUMPFORCE + 130; // 100
			playerWidth = 8; playerHeight = 8;
		}

			timer += dt;
			velocityY += player.gravity * dt;
			player.y += velocityY * dt;
			//speed = 100.0f;
			player.grounded = 0;

			if(IsKeyPressed(KEY_SPACE)) jumpBuffer = 0.1f;
			if(!player.groundPounding && IsKeyPressed(KEY_S)) groundPoundBuffer = 0.1f;
			if(player.groundPounding && IsKeyPressed(KEY_W)) {groundPoundBuffer = 0.0f; velocityY = 0.0f; groundPoundTimer = 0.0f; player.groundPounding = 0;}
			if(twirlCooldown == 0 && !player.grounded && !player.twirling && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				if(player.powerup == MINI) PlaySound(sounds.miniTwirlSound);
				else PlaySound(sounds.twirlSound);
				player.twirling = 1;
				twirlCooldown = TWIRL_COOLDOWN;
			} //twirlBuffer = 0.1f;

			if(jumpBuffer > 0) jumpBuffer -= dt;
			if(groundPoundBuffer > 0) groundPoundBuffer -= dt;
			if(groundPoundTimer > 0) groundPoundTimer -= dt;
			twirlTimer += dt;
			if(twirlCooldown > 0) twirlCooldown -= dt;
			else if(twirlCooldown < 0) twirlCooldown = 0.0f;


			BeginDrawing();

			ClearBackground(BACKGROUND_COLOR_GLOBAL);

			DrawTextureRec(bg, (Rectangle){0, 0, LEVEL_WIDTH_PIXEL, LEVEL_HEIGHT_PIXEL}, (Vector2){0, 0}, WHITE);

			for(int x = 0; x < LEVEL_WIDTH; x++) {
				for(int y = 0; y < LEVEL_HEIGHT; y++) {
					int tile_texture = level[y][x].sprite;
					if(tile_texture != -1 && tile_texture != 255)
						DrawTextureRec(
							tileset1,
							(Rectangle){tile_texture*16, 0, 16, 16},
							(Vector2){x*16, y*16},
							WHITE
						);
				}
			}

			// POWERUP UPDATE
			{
				int eggFound = 0;
				int JBFound = 0;

				for(int i = 0; i < MAX_POWERUP_SPAWN; i++) {
					// egg
					if(!usedEggPowerup[i] && !eggFound) {
						eggFound = 1;
						freeEggPowerupSlot = i;
					}
					else if(usedEggPowerup[i]) {
						updateAI(&eggPowerupAI[i], eggPowerupTX, &eggPowerupPos[i].x, &eggPowerupPos[i].y, EGG_POWERUP_WIDTH, EGG_POWERUP_HEIGHT, 0, player.x, player.y, dt);
					}

					// jump boost
					if(!usedJBPowerup[i] && !JBFound) {
						JBFound = 1;
						freeJBPowerupSlot = i;
					}
					else if(usedJBPowerup[i]) {
						updateAI(&JBPowerupAI[i], JBPowerupTX, &JBPowerupPos[i].x, &JBPowerupPos[i].y, JB_POWERUP_WIDTH, JB_POWERUP_HEIGHT, 0, player.x, player.y, dt);
					}
				}
				if(advanceEggPowerup > MAX_POWERUP_SPAWN) advanceEggPowerup = 0;
				if(advanceJBPowerup > MAX_POWERUP_SPAWN) advanceJBPowerup = 0;

				if(!eggFound) freeEggPowerupSlot = advanceEggPowerup;
				if(!JBFound) freeJBPowerupSlot = advanceJBPowerup;
			}
			
			//FALL
			checkCollisionOfSubject(SUBJECT_BOTTOM, &player.x, &player.y, playerWidth, playerHeight, &velocityY, &velocityX, &player, sounds, Object, 1, &__testAI);
			
			// HIT CEILING (PART OF JUMP)
			checkCollisionOfSubject(SUBJECT_TOP, &player.x, &player.y, playerWidth, playerHeight, &velocityY, &velocityX, &player, sounds, Object, 1, &__testAI);
			
			// JUMP
			if(jumpBuffer > 0 && player.grounded) {
				if(player.powerup == MINI) PlaySound(sounds.miniJumpSound);
				else PlaySound(sounds.jumpSound);
				velocityY = jumpForce;
				player.grounded = 0;
				jumpBuffer = 0;
				//fixingDoubleJumpIssue = 1;
			}
			// DOUBLE JUMP
			else if(jumpBuffer > 0 && player.doubleJump) {
				PlaySound(sounds.doubleJumpSound);
				velocityY = jumpForce;
				player.doubleJump = 0;
				jumpBuffer = 0;
			}

			// friction slide direction stuff
			if(velocityX > minSpeed && !player.frictionSliding) {
				if(facingLeft && IsKeyDown(KEY_D)) {
					if(player.grounded) {
						player.frictionSliding = 1;
						player.frictionSlideLeft = 1;
					}
					else {
						velocityX = minSpeed;
					}
				}
				else if(!facingLeft && IsKeyDown(KEY_A)) {
					if(player.grounded) {
						player.frictionSliding = 1;
						player.frictionSlideLeft = 0;
					}
					else {
						velocityX = minSpeed;
					}
				}
			}

			// slowing down direction stuff
			if(velocityX > minSpeed && !player.slowingDown &&
			  !(IsKeyDown(KEY_A) || IsKeyDown(KEY_D))) {
				if(facingLeft) {
					player.slowingDown = 1;
					player.slowDownLeft = 1;
				}
				else if(!facingLeft) {
					player.slowingDown = 1;
					player.slowDownLeft = 0;
				}
			}

			// move sideways
			if(!player.groundPounding || !(player.crouching && player.grounded)) {
				// GO LEFT
				if(IsKeyDown(KEY_A) && !player.frictionSliding) {
					//if(!facingLeft) velocityX = minSpeed;
					if(IsKeyDown(KEY_LEFT_SHIFT)) velocityX += accelXGain;
					else velocityX -= accelXLoss;
					player.x -= velocityX * dt;
					facingLeft = 1;

					if(player.x < 0) player.x = 0;
				}

				// GO RIGHT
				if(IsKeyDown(KEY_D) && !player.frictionSliding) {
					//if(facingLeft) {velocityX = minSpeed;}
					if(IsKeyDown(KEY_LEFT_SHIFT)) velocityX += accelXGain;
					else velocityX -= accelXLoss;
					player.x += velocityX * dt;
					facingLeft = 0;

					if(player.x > LEVEL_WIDTH_PIXEL - playerWidth)
						player.x = LEVEL_WIDTH_PIXEL - playerWidth;
				}

			}
			
			checkCollisionOfSubject(SUBJECT_LEFT, &player.x, &player.y, playerWidth, playerHeight, &velocityY, &velocityX, &player, sounds, Object, 1, &__testAI);
			checkCollisionOfSubject(SUBJECT_RIGHT, &player.x, &player.y, playerWidth, playerHeight, &velocityY, &velocityX, &player, sounds, Object, 1, &__testAI);

			if(!player.grounded) player.frictionSliding = 0;

			// keep acceleration (friction slide)
			if(player.frictionSliding && player.grounded) {
				if(player.powerup == MINI) PlaySound(sounds.miniFrictionSlideSound);
				else PlaySound(sounds.doubleJumpSound);
				velocityX -= accelXLoss;
				if(velocityX <= minSpeed) {
					velocityX = minSpeed;
					player.frictionSliding = 0;
				}
				else {
					if(player.frictionSlideLeft) {
						player.x -= (velocityX-frictionSlideAccelXLoss) * dt;
						checkCollisionOfSubject(SUBJECT_LEFT, &player.x, &player.y, playerWidth, playerHeight, &velocityY, &velocityX, &player, sounds, Object, 1, &__testAI);
					}
					else {
						player.x += (velocityX-frictionSlideAccelXLoss) * dt;
						checkCollisionOfSubject(SUBJECT_RIGHT, &player.x, &player.y, playerWidth, playerHeight, &velocityY, &velocityX, &player, sounds, Object, 1, &__testAI);
					}
				}
			}
			else if(player.slowingDown) {
				velocityX -= accelXLoss;

				if(velocityX <= minSpeed) {
					velocityX = minSpeed;
					player.slowingDown = 0;
				}
				else {
					if(player.slowDownLeft) {
						player.x -= (velocityX-frictionSlideAccelXLoss) * dt;
						checkCollisionOfSubject(SUBJECT_LEFT, &player.x, &player.y, playerWidth, playerHeight, &velocityY, &velocityX, &player, sounds, Object, 1, &__testAI);
					}
					else {
						player.x += (velocityX-frictionSlideAccelXLoss) * dt;
						checkCollisionOfSubject(SUBJECT_RIGHT, &player.x, &player.y, playerWidth, playerHeight, &velocityY, &velocityX, &player, sounds, Object, 1, &__testAI);
					}
				}
			}

			if(velocityX > maxSpeed) velocityX = maxSpeed;
			if(velocityX < minSpeed) velocityX = minSpeed;

			// GROUND POUND
			if(groundPoundBuffer > 0 && !player.grounded) {
				groundPoundTimer = GROUND_POUND_STALL_TIME;
				player.groundPounding = 1;
				groundPoundBuffer = 0;
			}

			if(player.groundPounding) {
				if(groundPoundTimer > 0) {
					velocityY = -player.gravity * dt;
				}
				else velocityY = groundPoundForce;
				velocityX = 0;
			}

			//if(twirlBuffer > 0 && !player.grounded) {
				//player.twirling = 1;
				//twirlBuffer = 0;
			//}

			// walking animation
			if(timer >= 0.15f)
			{
				timer = 0.0f;

				if(frame == 1) frame = 2;
				else frame = 1;

				if(player.invincible) {
					invinc_frame = !invinc_frame;
				}

				/*if(player.groundPounding && groundPoundTimer > 0)
					playerRotation += 90.0f;
				if(playerRotation >= 360.0f) playerRotation = 0;*/
			}

			if(player.grounded) {
				twirlCooldown = 0.0f;
				twirlTimer = 0.0f;
				player.twirling = 0;
			}

			if(twirlTimer >= 0.3f) {
				twirlTimer = 0.0f;
				player.twirling = 0;
			}
			else if(player.twirling) {
				velocityY = -player.gravity * dt;
				twirlFrame = -twirlFrame;
			}

			if(player.invincible > 0) player.invincible -= dt;
			if(player.invincible < 0) player.invincible = 0.0f;
			if(player.invincible == 0) invinc_frame = 0;

			if(player.y > LEVEL_WIDTH_PIXEL) {
				player.dead = 1;
			}



			float playerWidthMirror = facingLeft ? -playerWidthTexture : playerWidthTexture;
			if(player.twirling) playerWidthMirror = twirlFrame; // how the hell does this work lol

			if(player.frictionSliding) {
				playerWidthMirror = player.frictionSlideLeft ? -playerWidthTexture : playerWidthTexture;		
			}

			int walking = IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || player.slowingDown;
			int spriteFrameX = 0;
			int spriteFrameY = 0;
			int spriteScaleX = 0;
			int spriteScaleY = 0;
			float playerXTexture = player.x;
			Texture2D playerSprite = player.powerup == JUMP_BOOST ? playerJB : playerTX;
			
			if(player.powerup == SUPER || player.powerup ==  JUMP_BOOST) {
				if(player.groundPounding) spriteFrameX = 4;
				else if(!player.grounded) {
					if(player.twirling) spriteFrameX = 6; // player.twirling in air
					else spriteFrameX = 3; // jumping / in air
				}
				else if(walking) spriteFrameX = frame;
				else spriteFrameX = 0;
				
				if(player.frictionSliding) spriteFrameX = 5;
				spriteScaleX = 16;
				spriteScaleY = 32;
				playerXTexture -= 1;
			}
			else if(player.powerup == MINI) {
				if(player.groundPounding) {spriteFrameX = 24; spriteFrameY = 2;}
				else if(!player.grounded) {
					if(player.twirling) {spriteFrameX = 24; spriteFrameY = 3;} // player.twirling in air
					else {spriteFrameX = 24; spriteFrameY = 1;}; // jumping / in air
				}
				else if(walking) {spriteFrameX = 25; spriteFrameY = frame-1;}
				else {spriteFrameX = 24; spriteFrameY = 0;}
				
				if(player.frictionSliding) {spriteFrameX = 25; spriteFrameY = 2;}
				spriteScaleX = 8;
				spriteScaleY = 8;
			}
			else if(player.powerup == SMALL) {
				if(player.groundPounding) {spriteFrameX = 9; spriteFrameY = 1;}
				else if(!player.grounded) {
					if(player.twirling) {spriteFrameX = 10; spriteFrameY = 1;} // player.twirling in air
					else {spriteFrameX = 9; spriteFrameY = 0;} // jumping / in air
				}
				else if(walking) {spriteFrameX = 8; spriteFrameY = frame-1;}
				else {spriteFrameX = 7; spriteFrameY = 1;}
				
				if(player.frictionSliding) {spriteFrameX = 10; spriteFrameY = 0;}
				spriteScaleX = 16;
				spriteScaleY = 16;
			}

			//updateAI(&__testAI, hurtle, &hurtleX, &hurtleY, hurtleW, hurtleH, frame, player.x, player.y, dt);

			for(int i = 0; i < SPRITE_LIMIT; i++) {
				if(!sprites[i].unused) {
					updateAI(&sprites[i].AI, textures[sprites[i].textureID], &sprites[i].x, &sprites[i].y, sprites[i].width, sprites[i].height, frame, player.x, player.y, dt);
				}
			}

			if(!invinc_frame)
				DrawTexturePro(
					playerSprite,
					(Rectangle){spriteFrameX*spriteScaleX, spriteFrameY*spriteScaleY, playerWidthMirror, playerHeight},
					(Rectangle){playerXTexture, player.y, playerWidthTexture, playerHeight},
					(Vector2){0, 0},
					0.0f, WHITE);

			for(int i = 0; i < 2; i++) {
				DrawTexturePro(
					objects,
					(Rectangle){0, 0, 16, 16},
					(Rectangle){Object[i].jumpBoostX*16, Object[i].jumpBoostY*16, 16, 16},
					(Vector2){0, 0},
					0.0f, WHITE);
			}

			//				  ===== DEBUG =====
			//DrawText(TextFormat("GROUNDED: %d", grounded), 10, 10, 20, RED);
			//DrawText(TextFormat("VEL Y: %.2f", velocityY), 10, 30, 20, RED);
			//DrawText(TextFormat("PLAYER Y: %.2f", player.y), 10, 50, 20, RED);
			//DrawText(TextFormat("INVINCIBLE: %.2f", invincible), 10, 70, 20, RED);
			//DrawText(TextFormat("VEL X: %.2f", velocityX), 10, 10, 20, RED);
			//DrawText(TextFormat("HURTLE X: %.2f", hurtleX), 10, 10, 20, RED);


		 if(player.dead) {
			if(!playerDeadSoundPlayed) {
				PlaySound(sounds.playerDeadSound);
				playerDeadSoundPlayed = 1;
			}
			DrawRectangle(90, 186, 350, 120, WHITE);
			DrawText("YOU DIED!", 110, 200, 60, RED);
			DrawText("Press ESC to quit.", 130, 270, 30, RED);
			player.y = 600;
			velocityY = 0;
			if(IsKeyPressed(KEY_ESCAPE))
				break;
		 }
		EndDrawing();
	}	
	
	UnloadMusicStream(levelMusic);

	UnloadSound(sounds.miniFrictionSlideSound);
	UnloadSound(sounds.miniJumpSound);
	UnloadSound(sounds.miniTwirlSound);
	UnloadSound(sounds.twirlSound);

	UnloadSound(sounds.playerDeadSound);
	UnloadSound(sounds.blockPowerUpThrowSound);
	UnloadSound(sounds.powerUpSound);
	UnloadSound(sounds.playerHurtSound);
	UnloadSound(sounds.doubleJumpSound);
	UnloadSound(sounds.jumpSound);

	for(int i = 0; i < textureCount; i++) {
		UnloadTexture(textures[i]);
	}
	MemFree(textures);

	UnloadTexture(objects);
	UnloadTexture(tileset1);
	UnloadTexture(playerJB);
	UnloadTexture(playerTX);
	UnloadTexture(bg);

	CloseAudioDevice();

	CloseWindow();

	return 0;
}