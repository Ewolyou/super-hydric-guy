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