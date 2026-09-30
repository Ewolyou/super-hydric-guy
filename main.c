
#include <raylib.h>
#include "level.h"

#define SMALL NORMAL
#define NORMAL 0
#define SUPER 1
#define JUMP_BOOST 2
#define MINI 3
#define DMG_INVINC_TIME 2
#define GROUND_POUND_STALL_TIME 0.25f
#define TWIRL_COOLDOWN 0.53f
#define PLAYER_JUMPFORCE -350.0f
#define PLAYER_GRAVITY 800.0f
#define SPRITE_GRAVITY 199.99f // 199.99f (formerly 207.33f idk why this happened) is the "max" before collision breaks for some reason, might fix later

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
	//int movedX; // set to 0 - counter for how many tiles it has moved horizontally
	//int movedY; // set to 0 - counter for how many tiles it has moved vertically
	int directionLR; // set to 0 - handled by updateAI() dev note: (0==LEFT,1==RIGHT)
	int directionUD; // set to 0 - handled by updateAI() dev note: (0==UP,1==DOWN)
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
	float rotating; // <0 = anti-clockwise, 0 = off, >0 = clockwise, value determines speed (rotates n degrees per second), IDK: max anti-clockwise value = -180, max clockwise value = 180
	int floats;  //  will not be affected by gravity
	int noClip;  //   will not collide with the terrain (use with (int)floats otherwise it will just fall through the level)
	float gravity; // used when (int)floats is 0
	float rotation; // set to 0 ( handled by (float)rotating )
} SpriteAI;


int checkCollisionOfSubject(int side, float *subjectX, float *subjectY, float subjectWidth, float subjectHeight, float *velocityY, struct Misc *misc, struct Sounds sounds, struct Obj Object[2], int isPlayer, SpriteAI *AI)
{
	int leftTile = (int)*subjectX / 16;
	int rightTile = (int)(*subjectX + subjectWidth-1) / 16;

	int bottomTile = (int)(*subjectY + subjectHeight-1) / 16;
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
			if(solid) {
				*subjectY = Y2Tile * 16 - subjectHeight;
				rcode = 1;
				
				if(isPlayer) {
					misc->grounded = 1;
					misc->ground_pounding = 0;
					*velocityY = 0;
					if(misc->powerUp == JUMP_BOOST) misc->doubleJump = 1;
					
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

	struct Misc miscSprite = {0, 0, 0, 0, 0, 0, AI->gravity};
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

		if(AI->state.movesStartingLeft || AI->state.movesOneWayX < 0) AI->state.directionLR = 0;
		else if(AI->state.movesStartingRight || AI->state.movesOneWayX > 0) AI->state.directionLR = 1;

		if(AI->state.movesStartingUp || AI->state.movesOneWayY < 0) AI->state.directionUD = 0;
		else if(AI->state.movesStartingDown || AI->state.movesOneWayY > 0) AI->state.directionUD = 1;

		if(!AI->floats) AI->velocityY = 0;

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
		if(AI->state.movesStartingLeft < 0 || AI->state.movesStartingRight < 0) {
			if(AI->state.directionLR == 0)
				*posX -= AI->velocityX * dt;
			else
				*posX += AI->velocityX * dt;
		}
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
		if(AI->state.movesStartingUp < 0 || AI->state.movesStartingDown < 0) {
			if(AI->state.directionUD == 0)
				*posY -= AI->velocityY * dt;
			else
				*posY += AI->velocityY * dt;
		}
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
		if(playerX+0 < *posX) {//						LEFT
			*posX -= AI->velocityX * dt;
			AI->state.directionLR = 0;
		}
		else if(playerX-0 > *posX) {//					RIGHT
			*posX += AI->velocityX * dt;
			AI->state.directionLR = 1;
		}
	}

	if(AI->state.chasesY) {
		if(playerY+0 < *posY) {//						UP
			*posY -= AI->velocityY * dt;
			AI->state.directionUD = 0;
		}
		else if(playerY+0 > *posY) {//					DOWN
			*posY += AI->velocityY * dt;
			AI->state.directionUD = 1;
		}
	}

	if(!AI->floats) {
		AI->velocityY += miscSprite.gravity * dt;
		*posY += AI->velocityY * dt;
	}

	if(!AI->noClip) {
		//if(AI->velocityX > 0 && AI->state.directionLR == 0) {
			if(checkCollisionOfSubject(SUBJECT_LEFT, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0, AI))
				if(AI->state.movesStartingLeft || AI->state.movesStartingRight) {
					AI->state.directionLR = 1;
					AI->state.startX = *posX;
				}
		//}
		//else if(AI->velocityX > 0 && AI->state.directionLR == 1) {
			if(checkCollisionOfSubject(SUBJECT_RIGHT, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0, AI))
				if(AI->state.movesStartingLeft || AI->state.movesStartingRight) {
					AI->state.directionLR = 0;
					AI->state.startX = *posX;
				}
		//}

		if(checkCollisionOfSubject(SUBJECT_TOP, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0, AI))
			if(AI->state.movesStartingUp || AI->state.movesStartingDown) {
				AI->state.directionUD = 1;
				AI->state.startY = *posY;
			}
		
		if(checkCollisionOfSubject(SUBJECT_BOTTOM, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0, AI))
			if(AI->state.movesStartingUp || AI->state.movesStartingDown) {
				AI->state.directionUD = 0;
				AI->state.startY = *posY;
			}
	}
	float sourceWidth = AI->state.directionLR ? -width : width;

	//   				 ============ DEBUG ============
	//DrawText(TextFormat("directionLR: %d", AI->state.directionLR), 10, 10, 20, RED);
	
	DrawTexturePro(sprite, (Rectangle){spriteFrame*width, 0, sourceWidth, height}, (Rectangle){(*posX)+8, (*posY)+8, width, height}, (Vector2){width/2.0f, height/2.0f}/*(Vector2){0, 0}*/, AI->rotation, WHITE);
}

int main(void)
{
	InitWindow(LEVEL_WIDTH_PIXEL, LEVEL_HEIGHT_PIXEL, "Super Hydric Guy");
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

	Sound twirlSound = LoadSound("audio/sfx/twirl.wav");
	Sound miniTwirlSound = LoadSound("audio/sfx/mini_twirl.wav");
	Sound miniJumpSound = LoadSound("audio/sfx/mini_jump.wav");
	Sound miniFrictionSlideSound = LoadSound("audio/sfx/mini_friction_slide.wav");

	SetSoundVolume(sounds.jumpSound, 0.5f);
	SetSoundVolume(miniJumpSound, 0.5f);
	SetSoundVolume(miniFrictionSlideSound, 0.5f);

	Music forest_music = LoadMusicStream("audio/forest_music.ogg");

	PlayMusicStream(forest_music);

	//						 CX CY OX OY ML MR MU MD M  M  M  M  M
	AIState __testAIState = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//						AI State	HT HL HR HB iD MX MN VX  VY  HTH R  F   no clip	          GR        M
	SpriteAI __testAI = {__testAIState, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 0, 1, .noClip = 0, SPRITE_GRAVITY, 0};

	int frame = 0;
	float twirlFrame = 16.0f;
	float timer = 0.0f;

	float playerX = 0.0f;
	float playerY = 0.0f;

	for(int x = 0; x < LEVEL_WIDTH; x++) {
		for(int y = 0; y < LEVEL_HEIGHT; y++) {
			if(level[y][x].sprite == 255) {
				playerX = x*16;
				playerY = (y-4)*16;
			}
		}
	}

	float velocityX = 0.0f;
	float velocityY = 0.0f;

	float jumpForce = PLAYER_JUMPFORCE;
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
		.powerUp = SUPER,
		.invincible = 0.0f,
		.dead = 0,
		.gravity = PLAYER_GRAVITY
	};

	int friction_sliding = 0;
	int twirling = 0;

	float frictionSlideAccelXLoss = 50.0f;
	float accelXGain = 1.5f;
	float accelXLoss = 2.0f;

	float playerWidth = 16;
	float playerHeight = 32;
	float playerRotation = 0.0f;

	float hurtleX = 26*16; float hurtleY = 12*16;
	float hurtleW = 16; float hurtleH = 16;

	while(!WindowShouldClose())
	{
		UpdateMusicStream(forest_music);	
		
		float dt = GetFrameTime();
		//misc.powerUp = MINI;
		if(misc.powerUp == NORMAL) {
			misc.gravity = PLAYER_GRAVITY;
			jumpForce = PLAYER_JUMPFORCE;
			playerWidth = 16; playerHeight = 16;}
		else if(misc.powerUp == SUPER ||
		        misc.powerUp == JUMP_BOOST) {
					misc.gravity = PLAYER_GRAVITY;
					jumpForce = PLAYER_JUMPFORCE;
					playerWidth = 16; playerHeight = 32;}
		if(misc.powerUp == MINI) {
			misc.gravity = PLAYER_GRAVITY - 480.0f; // 410
			jumpForce = PLAYER_JUMPFORCE + 130; // 100
			playerWidth = 8; playerHeight = 8;
		}

			timer += dt;
			velocityY += misc.gravity * dt;
			playerY += velocityY * dt;
			//speed = 100.0f;
			misc.grounded = 0;

			if(IsKeyPressed(KEY_SPACE)) jumpBuffer = 0.1f;
			if(!misc.ground_pounding && IsKeyPressed(KEY_S)) groundPoundBuffer = 0.1f;
			if(misc.ground_pounding && IsKeyPressed(KEY_W)) {groundPoundBuffer = 0.0f; velocityY = 0.0f; groundPoundTimer = 0.0f; misc.ground_pounding = 0;}
			if(twirlCooldown == 0 && !misc.grounded && !twirling && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				if(misc.powerUp == MINI) PlaySound(miniTwirlSound);
				else PlaySound(twirlSound);
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
			checkCollisionOfSubject(SUBJECT_BOTTOM, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1, &__testAI);
			
			// HIT CEILING (PART OF JUMP)
			checkCollisionOfSubject(SUBJECT_TOP, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1, &__testAI);
			
			// JUMP
			if(jumpBuffer > 0 && misc.grounded) {
				if(misc.powerUp == MINI) PlaySound(miniJumpSound);
				else PlaySound(sounds.jumpSound);
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
				}

				// GO RIGHT
				if(IsKeyDown(KEY_D) && !friction_sliding) {
					//if(facingLeft) {velocityX = minSpeed;}
					if(IsKeyDown(KEY_LEFT_SHIFT)) velocityX += accelXGain;
					else velocityX -= accelXLoss;
					playerX += velocityX * dt;
					facingLeft = 0;

					if(playerX > LEVEL_WIDTH_PIXEL - playerWidth)
						playerX = LEVEL_WIDTH_PIXEL - playerWidth;
				}
			}
			
			checkCollisionOfSubject(SUBJECT_LEFT, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1, &__testAI);
			checkCollisionOfSubject(SUBJECT_RIGHT, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1, &__testAI);

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
				if(misc.powerUp == MINI) PlaySound(miniFrictionSlideSound);
				else PlaySound(sounds.doubleJumpSound);
				velocityX -= accelXLoss;
				if(velocityX <= minSpeed) {
					velocityX = minSpeed;
					friction_sliding = 0;
				}
				else {
					if(facingLeft) {
						playerX -= (velocityX-frictionSlideAccelXLoss) * dt;

						if(playerX < 0) playerX = 0;
						checkCollisionOfSubject(SUBJECT_LEFT, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1, &__testAI);
					}
					else if(!facingLeft) {
						playerX += (velocityX-frictionSlideAccelXLoss) * dt;

						if(playerX > LEVEL_WIDTH * 16 - playerWidth)
							playerX = LEVEL_WIDTH * 16 - playerWidth;
						checkCollisionOfSubject(SUBJECT_RIGHT, &playerX, &playerY, playerWidth, playerHeight, &velocityY, &misc, sounds, Object, 1, &__testAI);
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

			ClearBackground(BLACK);

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
			int spriteFrameX = 0;
			int spriteFrameY = 0;
			int spriteScaleX = 0;
			int spriteScaleY = 0;
			Texture2D playerSprite = misc.powerUp == JUMP_BOOST ? playerJB : player;
			
			if(misc.powerUp == SUPER || misc.powerUp ==  JUMP_BOOST) {
				if(misc.ground_pounding) spriteFrameX = 4;
				else if(!misc.grounded) {
					if(twirling) spriteFrameX = 6; // twirling in air
					else spriteFrameX = 3; // jumping / in air
				}
				else if(walking) spriteFrameX = frame;
				else spriteFrameX = 0;
				
				if(friction_sliding) spriteFrameX = 5;
				spriteScaleX = 16;
				spriteScaleY = 32;
			}
			else if(misc.powerUp == MINI) {
				if(misc.ground_pounding) {spriteFrameX = 24; spriteFrameY = 2;}
				else if(!misc.grounded) {
					if(twirling) {spriteFrameX = 24; spriteFrameY = 3;} // twirling in air
					else {spriteFrameX = 24; spriteFrameY = 1;}; // jumping / in air
				}
				else if(walking) {spriteFrameX = 25; spriteFrameY = frame-1;}
				else {spriteFrameX = 24; spriteFrameY = 0;}
				
				if(friction_sliding) {spriteFrameX = 25; spriteFrameY = 2;}
				spriteScaleX = 8;
				spriteScaleY = 8;
			}
			else if(misc.powerUp == SMALL) {
				if(misc.ground_pounding) spriteFrameX = 4;
				else if(!misc.grounded) {
					if(twirling) spriteFrameX = 6; // twirling in air
					else spriteFrameX = 3; // jumping / in air
				}
				else if(walking) spriteFrameX = frame;
				
				if(friction_sliding) spriteFrameX = 5;
				spriteScaleX = 16;
				spriteScaleY = 16;
			}

			updateAI(&__testAI, hurtle, &hurtleX, &hurtleY, hurtleW, hurtleH, frame, playerX, playerY, dt);

			if(!invinc_frame)
				DrawTexturePro(
					playerSprite,
					(Rectangle){spriteFrameX*spriteScaleX, spriteFrameY*spriteScaleY, playerWidthMirror, playerHeight},
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
			DrawText(TextFormat("HURTLE X: %.2f", hurtleX), 10, 10, 20, RED);


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

	UnloadSound(miniFrictionSlideSound);
	UnloadSound(miniJumpSound);
	UnloadSound(miniTwirlSound);
	UnloadSound(twirlSound);

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