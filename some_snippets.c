// OLD CHECKCOLLISION WITH CRAPPY SYSTEM
int checkCollisionOfSubject(int side, float *subjectX, float *subjectY, float subjectWidth, float subjectHeight, float *velocityY, float *velocityX, struct Player *player, struct Sounds sounds, struct Obj Object[2], int isPlayer, SpriteAI *AI)
{
	int leftTile = (int)*subjectX / 16;
	int rightTile = (int)(*subjectX + subjectWidth-1) / 16;

	int bottomTile = (int)(*subjectY + subjectHeight-1) / 16;
	int topTile = (int)*subjectY / 16;

	int bottomPixel = (int)*subjectY + subjectHeight;
	int bottomTileForBottom = bottomPixel / 16;

	Rectangle subjectRec = {*subjectX, *subjectY, subjectWidth, subjectHeight};
	Rectangle eggPowerupRec[MAX_POWERUP_SPAWN] = {0};
	Rectangle JBPowerupRec[MAX_POWERUP_SPAWN] = {0};

	for(int i = 0; i < MAX_POWERUP_SPAWN; i++) {
		eggPowerupRec[i].x = eggPowerupPos[i].x;
		eggPowerupRec[i].y = eggPowerupPos[i].y;
		eggPowerupRec[i].width = EGG_POWERUP_WIDTH;
		eggPowerupRec[i].height = EGG_POWERUP_HEIGHT;

		JBPowerupRec[i].x = JBPowerupPos[i].x;
		JBPowerupRec[i].y = JBPowerupPos[i].y;
		JBPowerupRec[i].width = JB_POWERUP_WIDTH;
		JBPowerupRec[i].height = JB_POWERUP_HEIGHT;
	}


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
					int spawnTwoEgg = 0; // STILL DOESNT WORK AHHHHHHHHHHHHHHHHHHHHHHHHHHH
					int spawnTwoJB = 0;
					if(level[Y1Tile][X1Tile].jumpBoost) {
						PlaySound(sounds.blockPowerUpThrowSound);

						if(player->powerup == MINI || player->powerup == SMALL) {
							spawnPowerup(EGG_POWERUP, X1Tile, Y1Tile-1);
							spawnTwoEgg = 1;
						}
						else {
							spawnPowerup(JB_POWERUP, X1Tile, Y1Tile-1);
							spawnTwoJB = 1;
						}

						level[Y1Tile][X1Tile].jumpBoost = 0;
						level[Y1Tile][X1Tile].sprite = 29;
					}
					if(level[Y1Tile][X2Tile].jumpBoost) {
						PlaySound(sounds.blockPowerUpThrowSound);

						if(spawnTwoEgg) advanceEggPowerup++;
						if(spawnTwoJB)  advanceJBPowerup++;

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
					if( CheckCollisionRecs(eggPowerupRec[i], subjectRec) ) {
							switchPowerup(player, 0, 0, SUPER, sounds);
							despawnPowerup(EGG_POWERUP, i);
					}
				}

				if(usedJBPowerup[i]) {
					if( CheckCollisionRecs(JBPowerupRec[i], subjectRec) ) {
							switchPowerup(player, 0, 1, JUMP_BOOST, sounds);
							despawnPowerup(JB_POWERUP, i);
					}
				}
			}

			if(level[Y1Tile][X1Tile].harmful ||
			level[Y2Tile][X2Tile].harmful) {
				if(!player->invincible) {
					switchPowerup(player, 1, 0, 0, sounds);
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
						misc.powerup = JUMP_BOOST;
						misc.doubleJump = 1;
						Object[0].jumpBoostX = -1;
						Object[0].jumpBoostY = -1;
					}
					if((Object[1].jumpBoostX == leftTile || Object[1].jumpBoostX == rightTile) && Object[1].jumpBoostY == topTile) {
						PlaySound(sounds.powerUpSound);
						misc.powerup = JUMP_BOOST;
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
							if(misc.powerup == NORMAL) misc.dead = 1;
							else {
								PlaySound(sounds.playerHurtSound);
								misc.powerup = NORMAL;
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



			
			/*if(AI->state.movesOneWayX < 0 || AI->state.chasesX || AI->state.movesStartingLeft || AI->state.movesStartingRight) {
			if(checkCollisionOfSubject(SUBJECT_LEFT, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0, AI))
				if(AI->state.movesStartingLeft || AI->state.movesStartingRight) {
					AI->state.directionLR = 1;
					AI->state.startX = *posX;
				}
		}
		else if(AI->state.movesOneWayX > 0 || AI->state.movesStartingLeft || AI->state.movesStartingRight) {
			if(checkCollisionOfSubject(SUBJECT_RIGHT, posX, posY, width, height, &AI->velocityY, &miscSprite, soundsSprite, objSprite, 0, AI))
				if(AI->state.movesStartingLeft || AI->state.movesStartingRight) {
					AI->state.directionLR = 0;
					AI->state.startX = *posX;
				}
		}*/

/*
void ExportListC(void) {
	FILE *list_export = fopen("AI_list.c", "w");
	if(!list_export) {printf("ERROR: AI list file not found?"); return;}
	fprintf(list_export, "#include \"level.h\"\n#include \"AI_list.h\"\n\n");
	fprintf(list_export, "AIState UnusedAIState = {0};\nSpriteAI UnusedAI = {UnusedAIState, 0};\n");

	fprintf(list_export, "AIState AIStates[AI_STATE_LIMIT] = {\n\t");
	for(int i = 0; i < AI_STATE_LIMIT-1; i++)
		fprintf(list_export, "UnusedAIState, ");
	fprintf(list_export, "UnusedAIState\n};\n");

	fprintf(list_export, "SpriteAI AIs[AI_LIMIT] = {\n\t");
	for(int i = 0; i < AI_LIMIT-1; i++)
		fprintf(list_export, "UnusedAI, ");
	fprintf(list_export, "UnusedAI\n};\n");

	fprintf(list_export, "char AIStateNames[AI_STATE_LIMIT][MAX_TEXT_CHAR] = {\n\t");
	for(int i = 0; i < AI_STATE_LIMIT-1; i++)
		fprintf(list_export, "UNUSED_TEXT, ");
	fprintf(list_export, "UNUSED_TEXT\n};\n");
	
	fprintf(list_export, "char AINames[AI_LIMIT][MAX_TEXT_CHAR] = {\n\t");
	for(int i = 0; i < AI_LIMIT-1; i++)
		fprintf(list_export, "UNUSED_TEXT, ");
	fprintf(list_export, "UNUSED_TEXT\n};");

	fclose(list_export);

	/*FILE *list_export_again = fopen("AI_list.c", "w");
	if(!list_export_again) {printf("ERROR: AI list file not found?"); return;}
	fprintf(list_export_again, "#include \"level.h\"\n#include \"AI_list.h\"\n\n");

	fprintf(list_export_again, "AIState AIStates[AI_STATE_LIMIT] = {\n\t");
	for(int i = 0; i < AI_STATE_LIMIT-1; i++)
		fprintf(list_export_again, "AIStates[%d], ", i);
	fprintf(list_export_again, "AIStates[%d]\n} ", AI_STATE_LIMIT-1);

	fprintf(list_export_again, "SpriteAI AIs[AI_LIMIT] = {\n\t");
	for(int i = 0; i < AI_LIMIT-1; i++)
		fprintf(list_export_again, "AIs[%d], ", i);
	fprintf(list_export_again, "AIs[%d]\n} ", AI_LIMIT-1);

	fprintf(list_export_again, "char AIStateNames[AI_STATE_LIMIT][MAX_TEXT_CHAR] = {\n\t");
	for(int i = 0; i < AI_STATE_LIMIT-1; i++)
		fprintf(list_export_again, "%s, ", AIStateNames[i]);
	fprintf(list_export_again, "%s\n} ", AIStateNames[AI_STATE_LIMIT-1]);
	
	fprintf(list_export_again, "char AINames[AI_LIMIT][MAX_TEXT_CHAR] = {\n\t");
	for(int i = 0; i < AI_LIMIT-1; i++)
		fprintf(list_export_again, "%s, ", AINames[i]);
	fprintf(list_export_again, "%s\n} ", AINames[AI_LIMIT-1]);

	fclose(list_export_again);
};
*/