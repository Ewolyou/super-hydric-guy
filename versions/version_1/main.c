
#include <raylib.h>
#include "level.h"

#define NORMAL 0
#define JUMP_BOOST 1
#define DMG_INVINC_TIME 2
#define GROUND_POUND_STALL_TIME 0.25f
#define TWIRL_COOLDOWN 0.53f

#define SUBJECT_BOTTOM 0
#define SUBJECT_TOP 1
#define SUBJECT_LEFT 2
#define SUBJECT_RIGHT 3

struct Sounds {
	Sound jumpSound;
	Sound doubleJumpSound;
	Sound playerHurtSound;
	Sound powerUpSound;
	Sound blockPowerUpThrowSound;
	Sound playerDeadSound;
};

struct Misc {
	int grounded;
	int ground_pounding;
	int doubleJump;
	int powerUp;
	float invincible;
	int dead;
	float gravity;
};

typedef struct {
	int chasesX; // chases the player in the X axis
	int chasesY; // chases the player in the Y axis [ USE WITH (int)floats !!! ]
	int movesOneWayX; // <0 = left, 0 = off, >0 = right (will not turn if it collides with a wall)
	int movesOneWayY; // <0 = up, 0 = off, >0 = down (will not turn if it collides with blocks)	[ USE WITH (int)floats !!! ]
	int movesStartingLeft; // <0 = indefinitely, 0 = off, >0 = how many tiles it will move (turns right, once it finishes moving through the tiles, or collides with a wall when noClip is 0)
	int movesStartingRight; // <0 = indefinitely, 0 = off, >0 = how many tiles it will move (turns left, once it finishes moving through the tiles, or collides with a wall when noClip is 0)
	int movesStartingUp; // <0 = indefinitely, 0 = off, >0 = how many tiles it will move (turns down, once it finishes moving through the tiles, or collides with blocks when noClip is 0) [ USE WITH (int)floats !!! ]
	int movesStartingDown; // <0 = indefinitely, 0 = off, >0 = how many tiles it will move (turns up, once it finishes moving through the tiles, or collides with blocks when noClip is 0) [ USE WITH (int)floats !!! ]
	int movedX; // set to 0 - counter for how many tiles it has moved horizontally
	int movedY; // set to 0 - counter for how many tiles it has moved vertically
	int directionLR; // set to 0 - handled by updateAI()
	int directionUD; // set to 0 - handled by updateAI()
	float startX; // set to 0 - handled by updateAI()
	float startY; // set to 0 - handled by updateAI()
	float notFirstCall; // set to 0 - handled by updateAI()
} AIState;

typedef struct {
	AIState state; // see (struct)AIState for details
	int vulnerableOT; // -1 = 
	int vulnerableOL; // hurts the player if he collides with sprite's left side
	int vulnerableOR; // hurts the player if he collides with sprite's right side
	int vulnerableOB; // hurts the player if he collides with sprite's bottom
	int instaDeath; // instantly kills the player regardless of his powerup state
	float maxSpeed;
	float minSpeed;
	//float width; // sprite width and its collision/hit box in pixels
	//float height; // sprite height and its collision/hit box in pixels
	float velocityX;
	float velocityY;
	int health; // unused for now
	//int facingLeft; // where the sprite faces
	float rotating; // <0 = anti-clockwise, 0 = off, >0 = clockwise, value determines speed (rotates n degrees per second), max anti-clockwise value = -180, max clockwise value = 180
	int floats;  //  will not be affected by gravity
	int noClip;  //   will not collide with the terrain (use with (int)floats otherwise it will just fall through the level)
	float rotation; // set to 0 ( handled by (float)rotating )
} SpriteAI;


int checkCollisionOfSubject(int side, float *subjectX, float *subjectY, float subjectWidth, float subjectHeight, float *velocityY, struct Misc *misc, struct Sounds sounds, struct Obj Object[2], int isPlayer)
{
	int leftTile = (int)*subjectX / 16;
	int rightTile = (int)(*subjectX + subjectWidth-1) / 16;

	int bottomTile = (int)(*subjectY + subjectHeight-2) / 16;
	int topTile = (int)*subjectY / 16;

	int bottomPixel = (int)*subjectY + subjectHeight;
	int bottomTileForBottom = bottomPixel / 16;

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
		X1Tile = leftTile;
		X2Tile = leftTile;
		Y1Tile = topTile;
		Y2Tile = bottomTile;
	}
	else if(side == SUBJECT_RIGHT) {
		X1Tile = rightTile;
		X2Tile = rightTile;
		Y1Tile = topTile;
		Y2Tile = bottomTile;
	}
	
	if(X1Tile >= 0 && X1Tile < LEVEL_WIDTH &&
	X2Tile >= 0 && X2Tile < LEVEL_WIDTH &&
 	Y1Tile >= 0 && Y1Tile < LEVEL_HEIGHT &&
	Y2Tile >= 0 && Y2Tile < LEVEL_HEIGHT)
	{
		if(side == SUBJECT_BOTTOM){
			if(isPlayer && !(*velocityY >= 0)) return 0;
			if(level[Y1Tile][X1Tile].solidOT ||
			level[Y2Tile][X2Tile].solidOT) {
				*subjectY = Y2Tile * 16 - subjectHeight;
				rcode = 1;
				if(isPlayer) {
					*velocityY = 0;
					misc->grounded = 1;
				}
				if(misc->powerUp == JUMP_BOOST) misc->doubleJump = 1;
			}

			if(isPlayer && misc->ground_pounding && (level[Y2Tile][X1Tile].jumpBoost || level[Y2Tile][X2Tile].jumpBoost)) {	
				if(level[Y2Tile][X1Tile].jumpBoost) {
					PlaySound(sounds.blockPowerUpThrowSound);
					Object[0].jumpBoostY = Y2Tile+1;
					Object[0].jumpBoostX = X1Tile;
					level[Y2Tile][X1Tile].jumpBoost = 0;
					level[Y2Tile][X1Tile].sprite = 29;
				}
				if(level[Y2Tile][X2Tile].jumpBoost) {
					PlaySound(sounds.blockPowerUpThrowSound);
					Object[1].jumpBoostY = Y2Tile+1;
					Object[1].jumpBoostX = X2Tile;
					level[Y2Tile][X2Tile].jumpBoost = 0;
					level[Y2Tile][X2Tile].sprite = 29;
				}
			}

			if(level[Y1Tile][X1Tile].solidOT ||
			level[Y2Tile][X2Tile].solidOT) {
				misc->ground_pounding = 0;
				rcode = 1;
			}
		}
		if(side == SUBJECT_TOP) {
			if(isPlayer && !(*velocityY < 0)) return 0;
			if(level[Y1Tile][X1Tile].solidOB ||
			level[Y2Tile][X2Tile].solidOB) {
				*subjectY = Y1Tile * 16 + 16;
				if(isPlayer) *velocityY = 50;
				rcode = 1;
			}

			if(isPlayer) {
				if(level[Y1Tile][X1Tile].jumpBoost || level[Y1Tile][X2Tile].jumpBoost) {
					if(level[Y1Tile][X1Tile].jumpBoost) {
						PlaySound(sounds.blockPowerUpThrowSound);
						Object[0].jumpBoostY = topTile-1;
						Object[0].jumpBoostX = leftTile;
						level[Y1Tile][X1Tile].jumpBoost = 0;
						level[Y1Tile][X1Tile].sprite = 29;
					}
					if(level[Y1Tile][X2Tile].jumpBoost) {
						PlaySound(sounds.blockPowerUpThrowSound);
						Object[1].jumpBoostY = topTile-1;
						Object[1].jumpBoostX = rightTile;
						level[Y1Tile][X2Tile].jumpBoost = 0;
						level[Y1Tile][X2Tile].sprite = 29;
					}
				}
			}
		}
		if(side == SUBJECT_LEFT) {
			if(level[Y1Tile][X1Tile].solidOR ||
			level[Y2Tile][X2Tile].solidOR) {
				*subjectX = X1Tile * 16 + 16;
				rcode = 1;
			}
		}
		if(side == SUBJECT_RIGHT){
			if(level[Y1Tile][X1Tile].solidOL ||
			level[Y2Tile][X2Tile].solidOL) {
				*subjectX = X2Tile * 16 - 16;
				rcode = 1;
			}
		}

		if(isPlayer) {
			if((Object[0].jumpBoostX == X1Tile || Object[0].jumpBoostX == X2Tile) &&
			(Object[0].jumpBoostY == Y1Tile || Object[0].jumpBoostY == Y2Tile)) {
				PlaySound(sounds.powerUpSound);
				misc->powerUp = JUMP_BOOST;
				misc->doubleJump = 1;
				Object[0].jumpBoostX = -1;
				Object[0].jumpBoostY = -1;
			}
			if((Object[1].jumpBoostX == X1Tile || Object[1].jumpBoostX == X2Tile) &&
			(Object[1].jumpBoostY == Y1Tile || Object[1].jumpBoostY == Y2Tile)) {
				PlaySound(sounds.powerUpSound);
				misc->powerUp = JUMP_BOOST;
				misc->doubleJump = 1;
				Object[1].jumpBoostX = -1;
				Object[1].jumpBoostY = -1;
			}

			if(level[Y1Tile][X1Tile].harmful ||
			level[Y2Tile][X2Tile].harmful) {
				if(!misc->invincible) {
					if(misc->powerUp == NORMAL) misc->dead = 1;
					else {
						PlaySound(sounds.playerHurtSound);
						misc->powerUp = NORMAL;
						misc->invincible = DMG_INVINC_TIME;
						misc->doubleJump = 0;
					}
				}
			}

			if(level[Y1Tile][X1Tile].instaDeath ||
			level[Y2Tile][X2Tile].instaDeath)
				misc->dead = 1;
		}
	}

	return rcode;
}

void updateAI(SpriteAI *AI, Texture2D sprite, float *posX, float *posY, float width, float height, int spriteFrame, float playerX, float playerY, float dt) {
	AI->rotation += AI->rotating * dt;
	if(AI->rotation >= 360 || AI->rotation <= -360) AI->rotation = 0;

	struct Misc miscSprite = {0, 0, 0, 0, 0, 0, 2.0f};
	struct Obj objSprite[2] = {0};
	struct Sounds soundsSprite = {
		0, //LoadSound("audio/sfx/jump.wav"),
		0, //LoadSound("audio/sfx/double_jump.wav"),
		0, //LoadSound("audio/sfx/player_hurt.wav"),
		0, //LoadSound("audio/sfx/powerup.wav"),
		0, //LoadSound("audio/sfx/block_powerup_throw.wav"),
		0 //LoadSound("audio/sfx/player_dead.wav")
	};

	if(!AI->state.notFirstCall) {
		AI->state.startX = *posX;
		AI->state.startY = *posY;

		if(AI->state.movesStartingLeft > 0) AI->state.directionLR = 0;
		else if(AI->state.movesStartingRight > 0) AI->state.directionLR = 1;

		if(AI->state.movesStartingUp > 0) AI->state.directionUD = 0;
		else if(AI->state.movesStartingDown > 0) AI->state.directionUD = 1;

		AI->state.notFirstCall = 1;
	}

	if(AI->state.movesOneWayX) {
		if(AI->state.movesOneWayX < 0) *posX -= AI->velocityX * dt;
		else if(AI->state.movesOneWayX > 0) *posX += AI->velocityX * dt;
	}
	if(AI->state.movesOneWayY) {
		if(AI->state.movesOneWayY < 0) *posY -= AI->velocityY * dt;
		else if(AI->state.movesOneWayY > 0) *posY += AI->velocityY * dt;
	}

	if(AI->state.movesStartingLeft || AI->state.movesStartingRight) {
		if(AI->state.movesStartingLeft < 0) {*posX -= AI->velocityX * dt;}
		else if(AI->state.movesStartingRight < 0) {*posX += AI->velocityX * dt;}
		else {
			float movesStartingDirection = AI->state.movesStartingLeft ? AI->state.movesStartingLeft : AI->state.movesStartingRight;
			
			float distance = movesStartingDirection * 16.0f;
			float targetX = AI->state.directionLR
				? AI->state.startX + distance
				: AI->state.startX - distance;

			if(AI->state.directionLR) {
				*posX += AI->velocityX * dt;
				if(*posX >= targetX) {
					*posX = targetX;
					AI->state.startX = *posX;
					AI->state.directionLR = 0;
				}
			}
			else {
				*posX -= AI->velocityX * dt;
				if(*posX <= targetX) {
					*posX = targetX;
					AI->state.startX = *posX;
					AI->state.directionLR = 1;
				}
			}
		}
	}

	if(AI->state.movesStartingUp || AI->state.movesStartingDown) {
		if(AI->state.movesStartingUp < 0) {*posY -= AI->velocityY * dt;}
		else if(AI->state.movesStartingDown < 0) {*posY += AI->velocityY * dt;}
		else {
			float movesStartingDirection = AI->state.movesStartingUp ? AI->state.movesStartingUp : AI->state.movesStartingDown;
			
			float distance = movesStartingDirection * 16.0f;
			float targetY = AI->state.directionUD
				? AI->state.startY + distance
				: AI->state.startY - distance;

			if(AI->state.directionUD) {
				*posY += AI->velocityY * dt;
				if(*posY >= targetY) {
					*posY = targetY;
					AI->state.startY = *posY;
					AI->state.directionUD = 0;
				}
			}
			else {
				*posY -= AI->velocityY * dt;
				if(*posY <= targetY) {
					*posY = targetY;
					AI->state.startY = *posY;
					AI->state.directionUD = 1;
				}
			}
		}
	}

	if(AI->state.chasesX) {
		if(playerX+0 < *posX) *posX -= AI->velocityX * dt;
		else if(playerX-0 > *posX) *posX += AI->velocityX * dt;
	}

	if(AI->state.chasesY) {
		if(playerY+0 < *posY) *posY -= AI->velocityY * dt;
		else if(playerY+0 > *posY) *posY += AI->velocityY * dt;
	}

	if(!AI->floats) {
		AI->velocityY += miscSprite.gravity * dt;
		*posY += AI->velocityY * dt;
	}

	if(!AI->noClip) {
		checkCollisionOfSubject(SUBJECT_BOTTOM, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0);
		checkCollisionOfSubject(SUBJECT_TOP, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0);
		if(checkCollisionOfSubject(SUBJECT_LEFT, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0)) if(AI->state.movesStartingLeft || AI->state.movesStartingRight) AI->state.directionLR = 1; //!AI->state.directionLR;
		if(checkCollisionOfSubject(SUBJECT_RIGHT, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0)) if(AI->state.movesStartingLeft || AI->state.movesStartingRight) AI->state.directionLR = 0; //!AI->state.directionLR;
	}

	float sourceWidth = AI->state.directionLR ? -width : width;

	//   				 ============ DEBUG ============
	//DrawText(TextFormat("directionLR: %d", AI->state.directionLR), 10, 10, 20, RED);
	
	DrawTexturePro(sprite, (Rectangle){spriteFrame*width, 0, sourceWidth, height}, (Rectangle){(*posX)+8, (*posY)+8, width, height}, (Vector2){width/2.0f, height/2.0f}, AI->rotation, WHITE);
}

int main(void)
{
	InitWindow(512, 512, "Super Hydric Guy");
	InitAudioDevice();

	SetTargetFPS(60);

	Texture2D bg = LoadTexture("gfx/forest_bg.png");
	Texture2D player = LoadTexture("gfx/player.png");
	Texture2D playerJB = LoadTexture("gfx/playerJB.png");
	Texture2D tileset1 = LoadTexture("gfx/forest_tileset.png");
	Texture2D objects = LoadTexture("gfx/objects.png");
	Texture2D hurtle = LoadTexture("gfx/sprites/hurtle.png");

	struct Sounds sounds = {
		LoadSound("audio/sfx/jump.wav"),
		LoadSound("audio/sfx/double_jump.wav"),
		LoadSound("audio/sfx/player_hurt.wav"),
		LoadSound("audio/sfx/powerup.wav"),
		LoadSound("audio/sfx/block_powerup_throw.wav"),
		LoadSound("audio/sfx/player_dead.wav")
	};

	Sound twirl_sound = LoadSound("audio/sfx/twirl.wav");

	SetSoundVolume(sounds.jumpSound, 0.5f);

	Music forest_music = LoadMusicStream("audio/forest_music.ogg");

	PlayMusicStream(forest_music);

	//							CX CY OX OY XL XR YU YD M  M  M  M  M  M  M
	AIState __idleFloatState = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//									AI State	HT HL HR HB iD MX MN VX  VY  HTH R  F   no clip     M
	SpriteAI __idleFloatClipAI = {__idleFloatState, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 0, 0, .noClip = 0, 0};

	int frame = 0;
	float twirlFrame = 16.0f;
	float timer = 0.0f;

	float playerX = 0.0f;
	float playerY = 0.0f;

	for(int x = 0; x < LEVEL_WIDTH; x++) {
		for(int y = 0; y < LEVEL_HEIGHT; y++) {
			if(level[y][x].sprite == 255) {
				playerX = x*16;
				playerY = y*16;
			}
		}
	}

	float velocityX = 0.0f;
	float velocityY = 0.0f;

	float jumpForce = -350.0f;
	float groundPoundForce = 600.0f;
	//float speed = 100.0f;
	float maxSpeed = 180.0f;
	float minSpeed = 80.0f;

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

	struct Misc misc = {
		.grounded = 0,
		.ground_pounding = 0,
		.doubleJump = 0,
		.powerUp = NORMAL,
		.invincible = 0.0f,
		.dead = 0,
		.gravity = 800.0f
	};

	int friction_sliding = 0;
	int twirling = 0;

	float frictionSlideAccelXLoss = 50.0f;
	float accelXGain = 1.5f;
	float accelXLoss = 2.0f;

	float playerWidth = 16;
	float playerHeight = 32;
	float playerRotation = 0.0f;

	float hurtleX = 25*16; float hurtleY = 18*16;

	while(!WindowShouldClose())
	{
		UpdateMusicStream(forest_music);	
		
		float dt = GetFrameTime();

			timer += dt;
			velocityY += misc.gravity * dt;
			playerY += velocityY * dt;
			//speed = 100.0f;
			misc.grounded = 0;

			if(IsKeyPressed(KEY_SPACE)) jumpBuffer = 0.1f;
			if(!misc.ground_pounding && IsKeyPressed(KEY_S)) groundPoundBuffer = 0.1f;
			if(misc.ground_pounding && IsKeyPressed(KEY_W)) {groundPoundBuffer = 0.0f; velocityY = 0.0f; groundPoundTimer = 0.0f; misc.ground_pounding = 0;}
			if(twirlCooldown == 0 && !misc.grounded && !twirling && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				PlaySound(twirl_sound);
				twirling = 1;
				twirlCooldown = TWIRL_COOLDOWN;
			} //twirlBuffer = 0.1f;

			if(jumpBuffer > 0) jumpBuffer -= dt;
			if(groundPoundBuffer > 0) groundPoundBuffer -= dt;
			if(groundPoundTimer > 0) groundPoundTimer -= dt;
			twirlTimer += dt;
			if(twirlCooldown > 0) twirlCooldown -= dt;
			else if(twirlCooldown < 0) twirlCooldown = 0.0f;

			//FALL
			checkCollisionOfSubject(SUBJECT_BOTTOM, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1);
			
			// HIT CEILING (PART OF JUMP)
			checkCollisionOfSubject(SUBJECT_TOP, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1);

			/*if(velocityY < 0) {
				int leftTile = (int)playerX / 16;
				int rightTile = (int)(playerX + 15) / 16;
				int topTile = (int)playerY / 16;

				if(topTile < LEVEL_HEIGHT) {
					if(level[topTile][leftTile].solidOB ||
					level[topTile][rightTile].solidOB) {
						playerY = topTile * 16 + 16;
						velocityY = 50.0f;
					}

					if((Object[0].jumpBoostX == leftTile || Object[0].jumpBoostX == rightTile) && Object[0].jumpBoostY == topTile) {
						PlaySound(sounds.powerUpSound);
						misc.powerUp = JUMP_BOOST;
						misc.doubleJump = 1;
						Object[0].jumpBoostX = -1;
						Object[0].jumpBoostY = -1;
					}
					if((Object[1].jumpBoostX == leftTile || Object[1].jumpBoostX == rightTile) && Object[1].jumpBoostY == topTile) {
						PlaySound(sounds.powerUpSound);
						misc.powerUp = JUMP_BOOST;
						misc.doubleJump = 1;
						Object[1].jumpBoostX = -1;
						Object[1].jumpBoostY = -1;
					}
					
					if(level[topTile][leftTile].jumpBoost || level[topTile][rightTile].jumpBoost) {
							
							if(level[topTile][leftTile].jumpBoost) {
								PlaySound(sounds.blockPowerUpThrowSound);
								Object[0].jumpBoostY = topTile-1;
								Object[0].jumpBoostX = leftTile;
								level[topTile][leftTile].jumpBoost = 0;
								level[topTile][leftTile].sprite = 29;
							}
							if(level[topTile][rightTile].jumpBoost) {
								PlaySound(sounds.blockPowerUpThrowSound);
								Object[1].jumpBoostY = topTile-1;
								Object[1].jumpBoostX = rightTile;
								level[topTile][rightTile].jumpBoost = 0;
								level[topTile][rightTile].sprite = 29;
							}
						}

					if(level[topTile][leftTile].harmful ||
					level[topTile][rightTile].harmful) {
						if(!misc.invincible) {
							if(misc.powerUp == NORMAL) misc.dead = 1;
							else {
								PlaySound(sounds.playerHurtSound);
								misc.powerUp = NORMAL;
								misc.invincible = DMG_INVINC_TIME;
								misc.doubleJump = 0;
							}
						}
					}

					if(level[topTile][leftTile].instaDeath ||
					level[topTile][rightTile].instaDeath) {
						misc.dead = 1;
					}
				}

				if(playerY < 0) playerY = 0;
			}*/
			
			// JUMP
			if(jumpBuffer > 0 && misc.grounded) {
				PlaySound(sounds.jumpSound);
				velocityY = jumpForce;
				misc.grounded = 0;
				jumpBuffer = 0;
				//fixingDoubleJumpIssue = 1;
			}
			// DOUBLE JUMP
			else if(jumpBuffer > 0 && misc.doubleJump) {
				PlaySound(sounds.doubleJumpSound);
				velocityY = jumpForce;
				misc.doubleJump = 0;
				jumpBuffer = 0;
			}
			
			if(!misc.grounded) friction_sliding = 0;
			
			if(!misc.ground_pounding) {
				// GO LEFT
				if(IsKeyDown(KEY_A) && !friction_sliding) {
				//if(!facingLeft) velocityX = minSpeed;
				if(IsKeyDown(KEY_LEFT_SHIFT)) velocityX += accelXGain;
				else velocityX -= accelXLoss;
				playerX -= velocityX * dt;
				facingLeft = 1;

				if(playerX < 0) playerX = 0;

				checkCollisionOfSubject(SUBJECT_LEFT, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1);
			}

				// GO RIGHT
				if(IsKeyDown(KEY_D) && !friction_sliding) {
				//if(facingLeft) {velocityX = minSpeed;}
				if(IsKeyDown(KEY_LEFT_SHIFT)) velocityX += accelXGain;
				else velocityX -= accelXLoss;
				playerX += velocityX * dt;
				facingLeft = 0;

				if(playerX > LEVEL_WIDTH * 16 - 16)
					playerX = LEVEL_WIDTH * 16 - 16;
				
				checkCollisionOfSubject(SUBJECT_RIGHT, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1);
			}
			}

			//if(!(IsKeyDown(KEY_A) || IsKeyDown(KEY_D))) velocityX -= accelXLoss;

			if(velocityX > maxSpeed) velocityX = maxSpeed;
			if(velocityX < minSpeed) velocityX = minSpeed;

			// keep acceleration (friction slide)
			if(velocityX > minSpeed &&
			(!(IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) ||
				(facingLeft && IsKeyDown(KEY_D)) ||
				(!facingLeft && IsKeyDown(KEY_A))))
					friction_sliding = 1;

			if(!misc.grounded) friction_sliding = 0;

			if(friction_sliding) {
				PlaySound(sounds.doubleJumpSound);
				velocityX -= accelXLoss;
				if(velocityX <= minSpeed) {
					velocityX = minSpeed;
					friction_sliding = 0;
				}
				else {
					if(facingLeft) {
						playerX -= (velocityX-frictionSlideAccelXLoss) * dt;

						if(playerX < 0) playerX = 0;
						checkCollisionOfSubject(SUBJECT_LEFT, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1);
					}
					else if(!facingLeft) {
						playerX += (velocityX-frictionSlideAccelXLoss) * dt;

						if(playerX > LEVEL_WIDTH * 16 - 16)
							playerX = LEVEL_WIDTH * 16 - 16;
						checkCollisionOfSubject(SUBJECT_RIGHT, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1);
					}
				}
			}

			// GROUND POUND
			if(groundPoundBuffer > 0 && !misc.grounded) {
				groundPoundTimer = GROUND_POUND_STALL_TIME;
				misc.ground_pounding = 1;
				groundPoundBuffer = 0;
			}

			if(misc.ground_pounding) {
				if(groundPoundTimer > 0) {
					velocityY = -misc.gravity * dt;
				}
				else velocityY = groundPoundForce;
			}

			//if(twirlBuffer > 0 && !misc.grounded) {
				//twirling = 1;
				//twirlBuffer = 0;
			//}

			// walking animation
			if(timer >= 0.15f)
			{
				timer = 0.0f;

				if(frame == 1) frame = 2;
				else frame = 1;

				if(misc.invincible) {
					invinc_frame = !invinc_frame;
				}

				/*if(misc.ground_pounding && groundPoundTimer > 0)
					playerRotation += 90.0f;
				if(playerRotation >= 360.0f) playerRotation = 0;*/
			}

			if(misc.grounded) {
				twirlCooldown = 0.0f;
				twirlTimer = 0.0f;
				twirling = 0;
			}

			if(twirlTimer >= 0.3f) {
				twirlTimer = 0.0f;
				twirling = 0;
			}
			else if(twirling) {
				velocityY = -misc.gravity * dt;
				twirlFrame = -twirlFrame;
			}

			if(misc.invincible > 0) misc.invincible -= dt;
			if(misc.invincible < 0) misc.invincible = 0.0f;
			if(misc.invincible == 0) invinc_frame = 0;

			if(playerY > 512.0f) {
				misc.dead = 1;
			}

			BeginDrawing();

			ClearBackground(RAYWHITE);

			DrawTexture(bg, 0, 0, WHITE);

			for(int x = 0; x < LEVEL_WIDTH; x++) {
				for(int y = 0; y < LEVEL_HEIGHT; y++) {
					int tset = level[y][x].sprite;
					if(tset != -1 && tset != 255)
						DrawTextureRec(
							tileset1,
							(Rectangle){tset*16, 0, 16, 16},
							(Vector2){x*16, y*16},
							WHITE
						);
				}
			}

			float playerWidthMirror = facingLeft ? -playerWidth : playerWidth;
			if(twirling) playerWidthMirror = twirlFrame;
			int walking = IsKeyDown(KEY_A) || IsKeyDown(KEY_D);
			int spriteFrame = 0;
			Texture2D playerSprite = misc.powerUp == JUMP_BOOST ? playerJB : player;
			
			if(misc.ground_pounding) spriteFrame = 4;
			else if(!misc.grounded) {
				if(twirling) spriteFrame = 6; // twirling in air
				else spriteFrame = 3; // jumping / in air
			}
			else if(walking) spriteFrame = frame;
			
			if(friction_sliding) spriteFrame = 5;

			updateAI(&__idleFloatClipAI, hurtle, &hurtleX, &hurtleY, 16, 32, frame, playerX, playerY, dt);
			
			if(!invinc_frame)
				DrawTexturePro(
					playerSprite,
					(Rectangle){spriteFrame*16, 0, playerWidthMirror, playerHeight},
					(Rectangle){playerX, playerY, playerWidth, playerHeight},
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
			//DrawText(TextFormat("PLAYER Y: %.2f", playerY), 10, 50, 20, RED);
			//DrawText(TextFormat("INVINCIBLE: %.2f", invincible), 10, 70, 20, RED);
			//DrawText(TextFormat("VEL X: %.2f", velocityX), 10, 10, 20, RED);


		 if(misc.dead) {
			if(!playerDeadSoundPlayed) {
				PlaySound(sounds.playerDeadSound);
				playerDeadSoundPlayed = 1;
			}
			DrawRectangle(90, 186, 350, 120, WHITE);
			DrawText("YOU DIED!", 110, 200, 60, RED);
			DrawText("Press ESC to quit.", 130, 270, 30, RED);
			playerY = 600;
			velocityY = 0;
			if(IsKeyPressed(KEY_ESCAPE))
				break;
		 }
		EndDrawing();
	}	
	
	UnloadMusicStream(forest_music);

	UnloadSound(twirl_sound);
	UnloadSound(sounds.playerDeadSound);
	UnloadSound(sounds.blockPowerUpThrowSound);
	UnloadSound(sounds.powerUpSound);
	UnloadSound(sounds.playerHurtSound);
	UnloadSound(sounds.doubleJumpSound);
	UnloadSound(sounds.jumpSound);

	UnloadTexture(objects);
	UnloadTexture(tileset1);
	UnloadTexture(playerJB);
	UnloadTexture(player);
	UnloadTexture(bg);

	CloseAudioDevice();

	CloseWindow();

	return 0;
}
