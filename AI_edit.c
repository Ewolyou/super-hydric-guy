//#define RAYGUI_IMPLEMENTATION
#include <raylib.h>
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>
#include "AI_list.h"
#include "level.h"
#include <string.h>

#define AI_EDIT_WINDOW_WIDTH 1300
#define AI_EDIT_WINDOW_HEIGHT 910

#define AI_TAB_WIDTH 30
#define AI_TAB_HEIGHT 20
#define AI_TAB_SCALE_RATE_X 4
#define AI_TAB_SCALE_RATE_Y 2.6f
#define AI_TAB_START_X 70
#define AI_TAB_START_Y 0
#define AI_TAB_END_X (AI_TAB_START_X+(AI_TAB_WIDTH*AI_TAB_SCALE_RATE_X))
#define AI_TAB_END_Y (AI_TAB_START_Y+(AI_TAB_HEIGHT*AI_TAB_SCALE_RATE_Y))

#define AI_INFO_START_X (SPLIT_LINE_X+AI_SELECTION_START_X)
#define AI_INFO_START_Y 50
#define AI_INFO_END_X (AI_EDIT_WINDOW_WIDTH-400)

#define AI_SELECTION_START_X 20
#define AI_SELECTION_START_Y 60
#define AI_SELECTION_END_X (SPLIT_LINE_X-20)
#define AI_SELECTION_END_Y ((AI_SELECTION_START_Y+(AI_LIMIT*AIFontSize))+0)

#define SPLIT_LINE_X ((AI_EDIT_WINDOW_WIDTH/2)-301)


AIState UnusedAIState = {0};
SpriteAI UnusedAI = {{0}, 0};

int AIEditMode = 1;
int selectedState = 0;
int selectedAI = 0;
//int selectedFlag = 0;

int noEdit = 0;

void WriteAIState(FILE *f, const AIState *state) {
	fprintf(f,
		"{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %f, %f, %d, %d, %d, %f, %f, %f, %f, \"%s\", \"%s\", \"%s\", \"%s\"}",
		state->chasesX, state->chasesY,
		state->movesOneWayX, state->movesOneWayY,
		state->movesStartingLeft, state->movesStartingRight,
		state->movesStartingUp, state->movesStartingDown,
		state->directionLR, state->directionUD,
		(double)state->startX, (double)state->startY,
		state->notFirstCall, state->wantsToGoLR, state->wantsToGoUD,
		(double)state->chaseOffsetLeft, (double)state->chaseOffsetRight,
		(double)state->chaseOffsetUp, (double)state->chaseOffsetDown,
		state->chaseOffsetLeftText, state->chaseOffsetRightText,
		state->chaseOffsetUpText, state->chaseOffsetDownText);
}

void WriteSpriteAI(FILE *f, const SpriteAI *AI) {
	WriteAIState(f, &AI->state);
	fprintf(f,
		", %d, %d, %d, %d, %d, %f, %f, %f, %f, %d, %f, %d, %d, %f, %f, %d, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %f, %f, %f, %f, \"%s\", \"%s\"",
		AI->vulnerableOT, AI->vulnerableOL,
		AI->vulnerableOR, AI->vulnerableOB,
		AI->instaDeath, (double)AI->maxSpeedX, (double)AI->minSpeedX,
		(double)AI->velocityX, (double)AI->velocityY, AI->health,
		(double)AI->rotating, AI->floats, AI->noClip,
		(double)AI->gravity, (double)AI->rotation, AI->stateID, AI->maxSpeedXText,
		AI->minSpeedXText, AI->accelXGainText, AI->accelYGainText, AI->rotatingText,
		AI->gravityText, AI->syncRotationAndDirection, AI->eggPowerup,
		AI->jumpBoostPowerup, AI->maxSpeedY, AI->minSpeedY, AI->accelXGain,
		AI->accelYGain, AI->maxSpeedYText, AI->minSpeedYText);
}

void ExportListC(void) {
	FILE *list_export = fopen("AI_list.c", "w");
	if(!list_export) {printf("ERROR: AI list file not found?"); return;}
	fprintf(list_export, "#include \"level.h\"\n#include \"AI_list.h\"\n\n");

	/*fprintf(list_export, "AIState UnusedAIState = ");
	WriteAIState(list_export, &UnusedAIState);

	fprintf(list_export, ";\nSpriteAI UnusedAI = {");
	WriteSpriteAI(list_export, &UnusedAI);
	*/
	
	fprintf(list_export, "\nAIState AIStates[AI_STATE_LIMIT] = {\n\t");
	for(int i = 0; i < AI_STATE_LIMIT; i++) {
		WriteAIState(list_export, &AIStates[i]);
		//WriteAIState(list_export, &UnusedAIState);
		fprintf(list_export, "%s", i + 1 == AI_STATE_LIMIT ? "\n" : ",\n\t");
	}

	fprintf(list_export, "};\n\nSpriteAI AIs[AI_LIMIT] = {\n\t");
	for(int i = 0; i < AI_LIMIT; i++) {
		fputc('{', list_export);
		WriteSpriteAI(list_export, &AIs[i]);
		//WriteSpriteAI(list_export, &UnusedAI);
		fputc('}', list_export);
		fprintf(list_export, "%s", i + 1 == AI_LIMIT ? "\n" : ",\n\t");
	}

	fprintf(list_export, "};\n\nchar AIStateNames[AI_STATE_LIMIT][MAX_TEXT_CHAR] = {\n\t");
	for(int i = 0; i < AI_STATE_LIMIT; i++) {
		fprintf(list_export, "\"%s\"", AIStateNames[i]);
		fprintf(list_export, "%s", i + 1 == AI_STATE_LIMIT ? "\n" : ",\n\t");
	}

	fprintf(list_export, "};\n\nchar AINames[AI_LIMIT][MAX_TEXT_CHAR] = {\n\t");
	for(int i = 0; i < AI_LIMIT; i++) {
		fprintf(list_export, "\"%s\"", AINames[i]);
		fprintf(list_export, "%s", i + 1 == AI_LIMIT ? "\n" : ",\n\t");
	}

	fprintf(list_export, "};\n");
	fclose(list_export);
}

int main(void) {
	InitWindow(AI_EDIT_WINDOW_WIDTH, AI_EDIT_WINDOW_HEIGHT, "Super Hydric Guy - AI Editor");
	GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
	SetTargetFPS(60);

	Texture2D AITab = LoadTexture("gfx/AI_editor/AI_TAB.png");
	Texture2D AIStateTab = LoadTexture("gfx/AI_editor/AI_STATE_TAB.png");
	Texture2D AITabFocused = LoadTexture("gfx/AI_editor/AI_TAB_FOCUSED.png");
	Texture2D AIStateTabFocused = LoadTexture("gfx/AI_editor/AI_STATE_TAB_FOCUSED.png");

	//int spriteFontSize = AI_EDIT_WINDOW_HEIGHT/SPRITE_LIMIT;
	int AIStateFontSize = (AI_EDIT_WINDOW_HEIGHT-90)/AI_STATE_LIMIT;
	int AIFontSize = (AI_EDIT_WINDOW_HEIGHT-90)/AI_LIMIT;
	int AIStateFontGap = AIStateFontSize+10;
	int AIFontGap = AIFontSize+10;

	int AINameBoxEdit = 0;
	Rectangle AINameBox = {AI_INFO_START_X, AI_INFO_START_Y-20, 250, 32};

	int AIStateBoxEdit = 0;
	int maxSpeedXEdit = 0;
	int minSpeedXEdit = 0;
	int maxSpeedYEdit = 0;
	int minSpeedYEdit = 0;
	int accelXGainEdit = 0;
	int accelYGainEdit = 0;
	int healthEdit = 0;
	int rotatingEdit = 0;
	int gravityEdit = 0;

	int chaseOffsetLeftEdit = 0;
	int chaseOffsetRightEdit = 0;
	int chaseOffsetUpEdit = 0;
	int chaseOffsetDownEdit = 0;

	int movesStartingLeftBoxEdit = 0;
	int movesStartingRightBoxEdit = 0;
	int movesStartingUpBoxEdit = 0;
	int movesStartingDownBoxEdit = 0;

	char LeftRightText[3][MAX_TEXT_CHAR] = {"Left", "Off", "Right"};
	char UpDownText[3][MAX_TEXT_CHAR] = {"Up", "Off", "Down"};
	char PlayerNPCText[4][MAX_TEXT_CHAR] = {"No", "Non-Player Only", "Player Only", "Player AND Non-Player"};
	char CWAntiCWText[3][MAX_TEXT_CHAR] = {"Clockwise", "Off", "Anti-Clockwise"};

	int showSaveMessage = 0;

	while(!WindowShouldClose()) {
		noEdit = !showSaveMessage;
		
		if((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S)) {
			ExportListC();
			showSaveMessage = 1;
		}

		Vector2 mouse = GetMousePosition();
		
		BeginDrawing();
		ClearBackground(RAYWHITE);

		/*for(int y = 0; y < AI_LIMIT; y++) {
			if(mouse.x >= AI_SELECTION_START_X &&
			mouse.x < AI_SELECTION_END_X &&
			mouse.y >= AI_SELECTION_START_Y+(y*AIFontSize) &&
			mouse.y < AI_SELECTION_END_Y) {
				DrawRectangleLines(AI_SELECTION_START_X, AI_SELECTION_START_Y+(y*AIFontSize), AI_SELECTION_END_X, AI_SELECTION_END_Y-(y*AIFontSize), BLACK);
				DrawText(TextFormat("MOUSE Y: %.2f  Y END: %d", mouse.y, AI_SELECTION_END_Y), 400, 20, 20, RED);
				DrawText(TextFormat("y end stuff: %d", AI_SELECTION_END_Y-(y*AIFontSize)), 400, 50, 20, RED);
			}

		}*/

		Rectangle AITabButton = {
			AI_TAB_START_X,
			AI_TAB_START_Y,
			AI_TAB_WIDTH * AI_TAB_SCALE_RATE_X,
			AI_TAB_HEIGHT * AI_TAB_SCALE_RATE_Y
		};

		Rectangle AIStateTabButton = {
			AI_TAB_START_X + AI_TAB_WIDTH * AI_TAB_SCALE_RATE_X,
			AI_TAB_START_Y,
			AI_TAB_WIDTH * AI_TAB_SCALE_RATE_X,
			AI_TAB_HEIGHT * AI_TAB_SCALE_RATE_Y
		};

		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, AITabButton) && noEdit) AIEditMode = 1;
		else if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, AIStateTabButton) && noEdit) AIEditMode = 0;
		
		Texture2D AITabFocus = AIEditMode ? AITabFocused : AITab;
		Texture2D AIStateTabFocus = !AIEditMode ? AIStateTabFocused : AIStateTab;

		DrawTexturePro(AITabFocus, (Rectangle){0, 0, AI_TAB_WIDTH, AI_TAB_HEIGHT},
							  (Rectangle){AI_TAB_START_X, AI_TAB_START_Y, AI_TAB_WIDTH*AI_TAB_SCALE_RATE_X, AI_TAB_HEIGHT*AI_TAB_SCALE_RATE_Y},
							  (Vector2){0, 0}, 0.0f, WHITE);
		DrawTexturePro(AIStateTabFocus, (Rectangle){0, 0, AI_TAB_WIDTH, AI_TAB_HEIGHT},
							  (Rectangle){AI_TAB_START_X+(AI_TAB_WIDTH*AI_TAB_SCALE_RATE_X), AI_TAB_START_Y,
										  AI_TAB_WIDTH*AI_TAB_SCALE_RATE_X, AI_TAB_HEIGHT*AI_TAB_SCALE_RATE_Y},
							  (Vector2){0, 0}, 0.0f, WHITE);
		
		if(AIEditMode && noEdit) { // ========================== AI EDIT OUTLINE HIGHLIGHT AND SELECT ==================
			DrawRectangleLines(AI_SELECTION_START_X, (selectedAI+2.4f)*AIFontSize, AI_SELECTION_END_X - AI_SELECTION_START_X, AIFontSize, BLUE);
			for(int y = 0; y < AI_LIMIT; y++) {
				int rowY = AI_SELECTION_START_Y + (y*AIFontSize);
				//DrawText(TextFormat("rowY: %d", rowY), 500, 500, 20, RED);
				if(mouse.x >= AI_SELECTION_START_X &&
				mouse.x < AI_SELECTION_END_X &&
				mouse.y >= rowY &&
				mouse.y < rowY + AIFontSize) {
					DrawRectangleLines(AI_SELECTION_START_X, rowY, AI_SELECTION_END_X - AI_SELECTION_START_X, AIFontSize, BLACK);
					if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) selectedAI = y;
				}
			}
		}
		else if(!AIEditMode && noEdit) { // ========================== STATE EDIT OUTLINE HIGHLIGHT AND SELECT ==================
			DrawRectangleLines(AI_SELECTION_START_X, (selectedState+2.4f)*AIStateFontSize, AI_SELECTION_END_X - AI_SELECTION_START_X, AIStateFontSize, BLUE);
			for(int y = 0; y < AI_STATE_LIMIT; y++) {
				int rowY = AI_SELECTION_START_Y + (y*AIStateFontSize);
				//DrawText(TextFormat("rowY: %d", rowY), 500, 500, 20, RED);
				if(mouse.x >= AI_SELECTION_START_X &&
				mouse.x < AI_SELECTION_END_X &&
				mouse.y >= rowY &&
				mouse.y < rowY + AIStateFontSize) {
					DrawRectangleLines(AI_SELECTION_START_X, rowY, AI_SELECTION_END_X - AI_SELECTION_START_X, AIStateFontSize, BLACK);
					if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) selectedState = y;
				}
			}
		}

		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && noEdit) { // ========================= AI/STATE TEXT BOX NAME EDIT ==================
			AINameBoxEdit = CheckCollisionPointRec(mouse, AINameBox);
		}
		
		if(!AIStateBoxEdit) AIs[selectedAI].state = AIStates[AIs[selectedAI].stateID];
		
		if(AIEditMode) { // =========================== ALL AI EDIT STUFF =============================
			GuiTextBox(AINameBox, AINames[selectedAI], sizeof(AINames[selectedAI]), AINameBoxEdit);

			Rectangle AIStateBox = {AI_INFO_START_X+860, AI_INFO_START_Y+AIFontGap, 50, AIFontSize-2};

			Rectangle vulnerableOTBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*2), AI_INFO_END_X, AIFontSize};
			Rectangle vulnerableOLBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*3), AI_INFO_END_X, AIFontSize};
			Rectangle vulnerableORBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*4), AI_INFO_END_X, AIFontSize};
			Rectangle vulnerableOBBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*5), AI_INFO_END_X, AIFontSize};

			Rectangle maxSpeedXBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*6), 250, AIFontSize};
			Rectangle minSpeedXBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*7), 250, AIFontSize};

			Rectangle maxSpeedYBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*8), 250, AIFontSize};
			Rectangle minSpeedYBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*9), 250, AIFontSize};

			Rectangle accelXGainBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*10), 250, AIFontSize};
			Rectangle accelYGainBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*11), 250, AIFontSize};

			Rectangle healthBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*12), 250, AIFontSize};

			Rectangle rotatingBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*13), 250, AIFontSize};
			Rectangle syncRotationAndDirectionBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*14), AI_INFO_END_X, AIFontSize};

			Rectangle floatsBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*15), AI_INFO_END_X, AIFontSize};
			Rectangle noClipBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*16), AI_INFO_END_X, AIFontSize};
			Rectangle gravityBox = {AI_INFO_START_X+650, AI_INFO_START_Y+(AIFontGap*17), 250, AIFontSize};

			Rectangle eggPowerupBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*18), AI_INFO_END_X, AIFontSize};
			Rectangle jumpBoostPowerupBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*19), AI_INFO_END_X, AIFontSize};

			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && noEdit) { // ========================= AISTATE ID EDIT ==================
				AIStateBoxEdit = CheckCollisionPointRec(mouse, AIStateBox);
				maxSpeedXEdit = CheckCollisionPointRec(mouse, maxSpeedXBox);
				minSpeedXEdit = CheckCollisionPointRec(mouse, minSpeedXBox);
				maxSpeedYEdit = CheckCollisionPointRec(mouse, maxSpeedYBox);
				minSpeedYEdit = CheckCollisionPointRec(mouse, minSpeedYBox);
				accelXGainEdit = CheckCollisionPointRec(mouse, accelXGainBox);
				accelYGainEdit = CheckCollisionPointRec(mouse, accelYGainBox);
				healthEdit = CheckCollisionPointRec(mouse, healthBox);
				rotatingEdit = CheckCollisionPointRec(mouse, rotatingBox);
				gravityEdit = CheckCollisionPointRec(mouse, gravityBox);
			}

			if(CheckCollisionPointRec(mouse, vulnerableOTBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, vulnerableOTBox.y-3, vulnerableOTBox.width, vulnerableOTBox.height+6, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					AIs[selectedAI].vulnerableOT++;
					if(AIs[selectedAI].vulnerableOT > 3) AIs[selectedAI].vulnerableOT = 0;
				}
			}
			else if(CheckCollisionPointRec(mouse, vulnerableOLBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, vulnerableOLBox.y-3, vulnerableOLBox.width, vulnerableOLBox.height+6, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					AIs[selectedAI].vulnerableOL++;
					if(AIs[selectedAI].vulnerableOL > 3) AIs[selectedAI].vulnerableOL = 0;
				}
			}
			else if(CheckCollisionPointRec(mouse, vulnerableORBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, vulnerableORBox.y-3, vulnerableORBox.width, vulnerableORBox.height+6, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					AIs[selectedAI].vulnerableOR++;
					if(AIs[selectedAI].vulnerableOR > 3) AIs[selectedAI].vulnerableOR = 0;
				}
			}
			else if(CheckCollisionPointRec(mouse, vulnerableOBBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, vulnerableOBBox.y-3, vulnerableOBBox.width, vulnerableOBBox.height+6, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					AIs[selectedAI].vulnerableOB++;
					if(AIs[selectedAI].vulnerableOB > 3) AIs[selectedAI].vulnerableOB = 0;
				}
			}
			else if(CheckCollisionPointRec(mouse, floatsBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, floatsBox.y-3, floatsBox.width, floatsBox.height+2, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					AIs[selectedAI].floats = !AIs[selectedAI].floats;
			}
			else if(CheckCollisionPointRec(mouse, noClipBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, noClipBox.y-3, noClipBox.width, noClipBox.height+2, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					AIs[selectedAI].noClip = !AIs[selectedAI].noClip;
			}
			else if(CheckCollisionPointRec(mouse, noClipBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, noClipBox.y-3, noClipBox.width, noClipBox.height+2, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					AIs[selectedAI].noClip = !AIs[selectedAI].noClip;
			}
			else if(CheckCollisionPointRec(mouse, syncRotationAndDirectionBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, syncRotationAndDirectionBox.y-3, syncRotationAndDirectionBox.width, syncRotationAndDirectionBox.height+2, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					AIs[selectedAI].syncRotationAndDirection = !AIs[selectedAI].syncRotationAndDirection;
			}
			else if(CheckCollisionPointRec(mouse, eggPowerupBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, eggPowerupBox.y-3, eggPowerupBox.width, eggPowerupBox.height+2, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					AIs[selectedAI].eggPowerup = !AIs[selectedAI].eggPowerup;
			}
			else if(CheckCollisionPointRec(mouse, jumpBoostPowerupBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, jumpBoostPowerupBox.y-3, jumpBoostPowerupBox.width, jumpBoostPowerupBox.height+2, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					AIs[selectedAI].jumpBoostPowerup = !AIs[selectedAI].jumpBoostPowerup;
			}

			DrawText(TextFormat("Used AI State ID (\"%s\"):", AIStateNames[AIs[selectedAI].stateID]), AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*1), AIFontSize-2, BLACK);
			GuiValueBox(AIStateBox, NULL, &AIs[selectedAI].stateID, 0, AI_STATE_LIMIT-1, AIStateBoxEdit);
			DrawText(TextFormat("Vulnerable On Top: %s", PlayerNPCText[AIs[selectedAI].vulnerableOT]), AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*2), AIFontSize, AIs[selectedAI].vulnerableOT ? DARKGREEN : RED);
			DrawText(TextFormat("Vulnerable On Left: %s", PlayerNPCText[AIs[selectedAI].vulnerableOL]), AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*3), AIFontSize, AIs[selectedAI].vulnerableOL ? DARKGREEN : RED);
			DrawText(TextFormat("Vulnerable On Right: %s", PlayerNPCText[AIs[selectedAI].vulnerableOR]), AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*4), AIFontSize, AIs[selectedAI].vulnerableOR ? DARKGREEN : RED);
			DrawText(TextFormat("Vulnerable On Bottom: %s", PlayerNPCText[AIs[selectedAI].vulnerableOB]), AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*5), AIFontSize, AIs[selectedAI].vulnerableOB ? DARKGREEN : RED);
			
			DrawText("Maximum Speed X (positive float value):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*6), AIFontSize, BLACK);
			//GuiTextBox(maxSpeedXBox, AIs[selectedAI].maxSpeedXText, sizeof(AIs[selectedAI].maxSpeedXText), maxSpeedXEdit);
			GuiValueBoxFloat(maxSpeedXBox, NULL, AIs[selectedAI].maxSpeedXText, &AIs[selectedAI].maxSpeedX, maxSpeedXEdit);
			
			DrawText("Minimum Speed X (positive float value):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*7), AIFontSize, BLACK);
			//GuiTextBox(minSpeedXBox, AIs[selectedAI].minSpeedXText, sizeof(AIs[selectedAI].minSpeedXText), minSpeedXEdit);
			GuiValueBoxFloat(minSpeedXBox, NULL, AIs[selectedAI].minSpeedXText, &AIs[selectedAI].minSpeedX, minSpeedXEdit);
			
			DrawText("Maximum Speed Y (positive float value):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*8), AIFontSize, BLACK);
			//GuiTextBox(maxSpeedYBox, AIs[selectedAI].maxSpeedYText, sizeof(AIs[selectedAI].maxSpeedYText), maxSpeedYEdit);
			GuiValueBoxFloat(maxSpeedYBox, NULL, AIs[selectedAI].maxSpeedYText, &AIs[selectedAI].maxSpeedY, maxSpeedYEdit);
			
			DrawText("Minimum Speed Y (positive float value):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*9), AIFontSize, BLACK);
			//GuiTextBox(minSpeedYBox, AIs[selectedAI].minSpeedYText, sizeof(AIs[selectedAI].minSpeedYText), minSpeedYEdit);
			GuiValueBoxFloat(minSpeedYBox, NULL, AIs[selectedAI].minSpeedYText, &AIs[selectedAI].minSpeedY, minSpeedYEdit);
			
			DrawText("Acceleration Gain X (positive float value):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*10), AIFontSize, BLACK);
			//GuiTextBox(accelXGainBox, AIs[selectedAI].accelXGainText, sizeof(AIs[selectedAI].), accelXGainEdit);
			GuiValueBoxFloat(accelXGainBox, NULL, AIs[selectedAI].accelXGainText, &AIs[selectedAI].accelXGain, accelXGainEdit);
			
			DrawText("Acceleration Gain Y (positive float value):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*11), AIFontSize, BLACK);
			//GuiTextBox(accelYGainBox, AIs[selectedAI].accelYGainText, sizeof(AIs[selectedAI].accelYGainText), accelYGainEdit);
			GuiValueBoxFloat(accelYGainBox, NULL, AIs[selectedAI].accelYGainText, &AIs[selectedAI].accelYGain, accelYGainEdit);
			
			DrawText("Health (unused lol):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*12), AIFontSize, BLACK);
			GuiValueBox(healthBox, NULL, &AIs[selectedAI].health, -1, LEVEL_WIDTH*2, healthEdit);
			
			int CWAntiCWTextValue = 1;
			if(AIs[selectedAI].rotating > 0) CWAntiCWTextValue = 0;
			else if(AIs[selectedAI].rotating == 0) CWAntiCWTextValue = 1;
			else if(AIs[selectedAI].rotating < 0) CWAntiCWTextValue = 2;
			DrawText(TextFormat("Rotating: %s (rotates %.2f degrees per second):", CWAntiCWText[CWAntiCWTextValue], AIs[selectedAI].rotating),
										AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*13), AIFontSize-5, AIs[selectedAI].rotating ? DARKGREEN : RED);
			//GuiTextBox(rotatingBox, AIs[selectedAI].rotatingText, sizeof(AIs[selectedAI].rotatingText), rotatingEdit);
			GuiValueBoxFloat(rotatingBox, NULL, AIs[selectedAI].rotatingText, &AIs[selectedAI].rotating, rotatingEdit);

			DrawText("Sync Rotation Direction And Movement Direction", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*14), AIFontSize, AIs[selectedAI].syncRotationAndDirection ? DARKGREEN : RED);
			DrawText("Floats (no gravity)", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*15), AIFontSize, AIs[selectedAI].floats ? DARKGREEN : RED);
			DrawText("No Clip (does not collide with tiles)", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*16), AIFontSize, AIs[selectedAI].noClip ? DARKGREEN : RED);
			
			DrawText("Gravity (<0 upward pull, >0 downward pull; float value):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*17), AIFontSize-1, BLACK);
			//GuiTextBox(gravityBox, AIs[selectedAI].gravityText, sizeof(AIs[selectedAI].gravityText), gravityEdit);
			GuiValueBoxFloat(gravityBox, NULL, AIs[selectedAI].gravityText, &AIs[selectedAI].gravity, gravityEdit);
			
			DrawText("Egg Powerup (doesn't despawn)", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*18), AIFontSize, AIs[selectedAI].eggPowerup ? DARKGREEN : RED);
			DrawText("Jump Boost Powerup (doesn't despawn)", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*19), AIFontSize, AIs[selectedAI].jumpBoostPowerup ? DARKGREEN : RED);

			/*if(!maxSpeedXEdit) {
				char *end;
				float value = strtof(AIs[selectedAI].maxSpeedXText, &end);

				if(end != AIs[selectedAI].maxSpeedXText && *end == '\0') {
					AIs[selectedAI].maxSpeedX = value;
				}
			}
			if(!minSpeedXEdit) {
				char *end;
				float value = strtof(AIs[selectedAI].minSpeedXText, &end);

				if(end != AIs[selectedAI].minSpeedXText && *end == '\0') {
					AIs[selectedAI].minSpeedX = value;
				}
			}
			
			if(!maxSpeedYEdit) {
				char *end;
				float value = strtof(AIs[selectedAI].maxSpeedYText, &end);

				if(end != AIs[selectedAI].maxSpeedYText && *end == '\0') {
					AIs[selectedAI].maxSpeedY = value;
				}
			}
			if(!minSpeedYEdit) {
				char *end;
				float value = strtof(AIs[selectedAI].minSpeedYText, &end);

				if(end != AIs[selectedAI].minSpeedYText && *end == '\0') {
					AIs[selectedAI].minSpeedY = value;
				}
			}

			if(!accelXGainEdit) {
				char *end;
				float value = strtof(AIs[selectedAI].accelXGainText, &end);

				if(end != AIs[selectedAI].accelXGainText && *end == '\0') {
					AIs[selectedAI].accelXGain = value;
				}
			}
			if(!accelYGainEdit) {
				char *end;
				float value = strtof(AIs[selectedAI].accelYGainText, &end);

				if(end != AIs[selectedAI].accelYGainText && *end == '\0') {
					AIs[selectedAI].accelYGain = value;
				}
			}
			
			if(!rotatingEdit) {
				char *end;
				float value = strtof(AIs[selectedAI].rotatingText, &end);

				if(end != AIs[selectedAI].rotatingText&& *end == '\0') {
					AIs[selectedAI].rotating = value;
				}
			}
			if(!gravityEdit) {
				char *end;
				float value = strtof(AIs[selectedAI].gravityText, &end);

				if(end != AIs[selectedAI].gravityText && *end == '\0') {
					AIs[selectedAI].gravity = value;
				}
			}*/

			//DrawText(TextFormat("max speed float: %f", AIs[selectedAI].maxSpeed), 800, 800, 20, RED);
		}
		
		else if(!AIEditMode) { // ====================================== ALL STATE EDIT STUFF ================================
			GuiTextBox(AINameBox, AIStateNames[selectedState], sizeof(AIStateNames[selectedState]), AINameBoxEdit);

			Rectangle chasesXBox = {AI_INFO_START_X, AI_INFO_START_Y+AIStateFontGap, AI_INFO_END_X, AIStateFontSize};
			Rectangle chasesYBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*2), AI_INFO_END_X, AIStateFontSize};

			Rectangle chaseOffsetLeftBox = {AI_INFO_START_X+500, AI_INFO_START_Y+(AIStateFontGap*3), 250, AIStateFontSize};
			Rectangle chaseOffsetRightBox = {AI_INFO_START_X+500, AI_INFO_START_Y+(AIStateFontGap*4), 250, AIStateFontSize};
			Rectangle chaseOffsetUpBox = {AI_INFO_START_X+500, AI_INFO_START_Y+(AIStateFontGap*5), 250, AIStateFontSize};
			Rectangle chaseOffsetDownBox = {AI_INFO_START_X+500, AI_INFO_START_Y+(AIStateFontGap*6), 250, AIStateFontSize};

			Rectangle movesOneWayXBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*7), AI_INFO_END_X, AIStateFontSize};
			Rectangle movesOneWayYBox = {AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*8), AI_INFO_END_X, AIStateFontSize};

			Rectangle movesStartingLeftBox = {AI_INFO_START_X+860, AI_INFO_START_Y+(AIStateFontGap*9), 50, AIStateFontSize};
			Rectangle movesStartingRightBox = {AI_INFO_START_X+860, AI_INFO_START_Y+(AIStateFontGap*10), 50, AIStateFontSize};
			Rectangle movesStartingUpBox = {AI_INFO_START_X+860, AI_INFO_START_Y+(AIStateFontGap*11), 50, AIStateFontSize};
			Rectangle movesStartingDownBox = {AI_INFO_START_X+860, AI_INFO_START_Y+(AIStateFontGap*12), 50, AIStateFontSize};

			if(CheckCollisionPointRec(mouse, chasesXBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, chasesXBox.y-3, chasesXBox.width, chasesXBox.height+2, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					AIStates[selectedState].chasesX = !AIStates[selectedState].chasesX;
			}

			else if(CheckCollisionPointRec(mouse, chasesYBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, chasesYBox.y-3, chasesYBox.width, chasesYBox.height+2, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					AIStates[selectedState].chasesY = !AIStates[selectedState].chasesY;
			}

			else if(CheckCollisionPointRec(mouse, movesOneWayXBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, movesOneWayXBox.y-3, movesOneWayXBox.width, movesOneWayXBox.height+6, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					AIStates[selectedState].movesOneWayX++;
					if(AIStates[selectedState].movesOneWayX > 1) AIStates[selectedState].movesOneWayX = -1;
				}
			}

			else if(CheckCollisionPointRec(mouse, movesOneWayYBox) && noEdit) {
				DrawRectangleLines(AI_INFO_START_X-10, movesOneWayYBox.y-3, movesOneWayYBox.width, movesOneWayYBox.height+6, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					AIStates[selectedState].movesOneWayY++;
					if(AIStates[selectedState].movesOneWayY > 1) AIStates[selectedState].movesOneWayY = -1;
				}
			}

			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && noEdit) {
				chaseOffsetLeftEdit = CheckCollisionPointRec(mouse, chaseOffsetLeftBox);
				chaseOffsetRightEdit = CheckCollisionPointRec(mouse, chaseOffsetRightBox);
				chaseOffsetUpEdit = CheckCollisionPointRec(mouse, chaseOffsetUpBox);
				chaseOffsetDownEdit = CheckCollisionPointRec(mouse, chaseOffsetDownBox);

				movesStartingLeftBoxEdit = CheckCollisionPointRec(mouse, movesStartingLeftBox);
				movesStartingRightBoxEdit = CheckCollisionPointRec(mouse, movesStartingRightBox);
				movesStartingUpBoxEdit = CheckCollisionPointRec(mouse, movesStartingUpBox);
				movesStartingDownBoxEdit = CheckCollisionPointRec(mouse, movesStartingDownBox);
			}

			DrawText("Chases (X)", AI_INFO_START_X, AI_INFO_START_Y+AIStateFontGap, AIStateFontSize, AIStates[selectedState].chasesX ? DARKGREEN : RED);
			DrawText("Chases (Y)", AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*2), AIStateFontSize, AIStates[selectedState].chasesY ? DARKGREEN : RED);

			DrawText("Chase Offset Left (float value, pixels):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*3), AIFontSize-1, BLACK);
			//GuiTextBox(chaseOffsetLeftBox, AIStates[selectedState].chaseOffsetLeftText, sizeof(AIStates[selectedState].chaseOffsetLeftText), chaseOffsetLeftEdit);
			GuiValueBoxFloat(chaseOffsetLeftBox, NULL, AIStates[selectedState].chaseOffsetLeftText, &AIStates[selectedState].chaseOffsetLeft, chaseOffsetLeftEdit);
			
			DrawText("Chase Offset Right (float value, pixels):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*4), AIFontSize-1, BLACK);
			//GuiTextBox(chaseOffsetRightBox, AIStates[selectedState].chaseOffsetRightText, sizeof(AIStates[selectedState].chaseOffsetRightText), chaseOffsetRightEdit);
			GuiValueBoxFloat(chaseOffsetRightBox, NULL, AIStates[selectedState].chaseOffsetRightText, &AIStates[selectedState].chaseOffsetRight, chaseOffsetRightEdit);
			
			DrawText("Chase Offset Up (float value, pixels):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*5), AIFontSize-1, BLACK);
			//GuiTextBox(chaseOffsetUpBox, AIStates[selectedState].chaseOffsetUpText, sizeof(AIStates[selectedState].chaseOffsetUpText), chaseOffsetUpEdit);
			GuiValueBoxFloat(chaseOffsetUpBox, NULL, AIStates[selectedState].chaseOffsetUpText, &AIStates[selectedState].chaseOffsetUp, chaseOffsetUpEdit);
			
			DrawText("Chase Offset Down (float value, pixels):", AI_INFO_START_X, AI_INFO_START_Y+(AIFontGap*6), AIFontSize-1, BLACK);
			//GuiTextBox(chaseOffsetDownBox, AIStates[selectedState].chaseOffsetDownText, sizeof(AIStates[selectedState].chaseOffsetDownText), chaseOffsetDownEdit);
			GuiValueBoxFloat(chaseOffsetDownBox, NULL, AIStates[selectedState].chaseOffsetDownText, &AIStates[selectedState].chaseOffsetDown, chaseOffsetDownEdit);


			DrawText(TextFormat("Moves One Way (X): %s", LeftRightText[AIStates[selectedState].movesOneWayX+1]), AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*7), AIStateFontSize, AIStates[selectedState].movesOneWayX ? DARKGREEN : RED);
			DrawText(TextFormat("Moves One Way (Y): %s", UpDownText[AIStates[selectedState].movesOneWayY+1]), AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*8), AIStateFontSize, AIStates[selectedState].movesOneWayY ? DARKGREEN : RED);

			DrawText("Moves Starting Left (bounces on walls, -1 = indefinitely, >0 = n tiles): ", AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*9), AIStateFontSize, AIStates[selectedState].movesStartingLeft ? DARKGREEN : RED);
			GuiValueBox(movesStartingLeftBox, NULL, &AIStates[selectedState].movesStartingLeft, -1, LEVEL_WIDTH*2, movesStartingLeftBoxEdit);
			DrawText("Moves Starting Right (bounces on walls, -1 = indefinitely, >0 = n tiles): ", AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*10), AIStateFontSize, AIStates[selectedState].movesStartingRight ? DARKGREEN : RED);
			GuiValueBox(movesStartingRightBox, NULL, &AIStates[selectedState].movesStartingRight, -1, LEVEL_WIDTH*2, movesStartingRightBoxEdit);
			DrawText("Moves Starting Up (bounces on walls, -1 = indefinitely, >0 = n tiles): ", AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*11), AIStateFontSize, AIStates[selectedState].movesStartingUp ? DARKGREEN : RED);
			GuiValueBox(movesStartingUpBox, NULL, &AIStates[selectedState].movesStartingUp, -1, LEVEL_WIDTH*2, movesStartingUpBoxEdit);
			DrawText("Moves Starting Down (bounces on walls, -1 = indefinitely, >0 = n tiles): ", AI_INFO_START_X, AI_INFO_START_Y+(AIStateFontGap*12), AIStateFontSize, AIStates[selectedState].movesStartingDown ? DARKGREEN : RED);
			GuiValueBox(movesStartingDownBox, NULL, &AIStates[selectedState].movesStartingDown, -1, LEVEL_WIDTH*2, movesStartingDownBoxEdit);

			/*if(!chaseOffsetLeftEdit) {
				char *end;
				float value = strtof(AIStates[selectedState].chaseOffsetLeftText, &end);

				if(end != AIStates[selectedState].chaseOffsetLeftText && *end == '\0') {
					AIStates[selectedState].chaseOffsetLeft = value;
				}
			}
			if(!chaseOffsetRightEdit) {
				char *end;
				float value = strtof(AIStates[selectedState].chaseOffsetRightText, &end);

				if(end != AIStates[selectedState].chaseOffsetRightText && *end == '\0') {
					AIStates[selectedState].chaseOffsetRight = value;
				}
			}
			if(!chaseOffsetUpEdit) {
				char *end;
				float value = strtof(AIStates[selectedState].chaseOffsetUpText, &end);

				if(end != AIStates[selectedState].chaseOffsetUpText && *end == '\0') {
					AIStates[selectedState].chaseOffsetUp = value;
				}
			}
			if(!chaseOffsetDownEdit) {
				char *end;
				float value = strtof(AIStates[selectedState].chaseOffsetDownText, &end);

				if(end != AIStates[selectedState].chaseOffsetDownText && *end == '\0') {
					AIStates[selectedState].chaseOffsetDown = value;
				}
			}*/
			
		}

		// SPLIT LINE IN THE MIDDLE
		DrawLine(SPLIT_LINE_X+1, 0, SPLIT_LINE_X+1, AI_EDIT_WINDOW_HEIGHT-40, BLACK);
		DrawLine(SPLIT_LINE_X, 0, SPLIT_LINE_X, AI_EDIT_WINDOW_HEIGHT-39, BLACK);

		// SPLIT LINE AT THE BOTTOM
		DrawLine(0, AI_EDIT_WINDOW_HEIGHT-40, AI_EDIT_WINDOW_WIDTH, AI_EDIT_WINDOW_HEIGHT-40, BLACK);
		DrawLine(0, AI_EDIT_WINDOW_HEIGHT-39, AI_EDIT_WINDOW_WIDTH, AI_EDIT_WINDOW_HEIGHT-39, BLACK);

		DrawText("[Ctrl+S] - Export (AI_list.c)", (AI_EDIT_WINDOW_WIDTH/3)-30, AI_EDIT_WINDOW_HEIGHT-33, 30, BLUE);

		if(AIEditMode) {
			for(int i = 0; i < AI_LIMIT; i++) {
				Color textColor = strcmp(AINames[i], UNUSED_TEXT) ? BLACK : LIGHTGRAY;
				DrawText(TextFormat("%-20s (ID %d)", AINames[i], i), AI_SELECTION_START_X+5, (AIFontSize*(i))+AI_SELECTION_START_Y, AIFontSize, textColor);
			}
		}
		else if(!AIEditMode) {
			for(int i = 0; i < AI_STATE_LIMIT; i++) {
				Color textColor = strcmp(AIStateNames[i], UNUSED_TEXT) ? BLACK : LIGHTGRAY;
				DrawText(TextFormat("%-20s (ID %d)", AIStateNames[i], i), AI_SELECTION_START_X+5, (AIStateFontSize*(i))+AI_SELECTION_START_Y, AIStateFontSize, textColor);
			}
		}

		if(!noEdit) {
			AIStateBoxEdit = 0;
			maxSpeedXEdit = 0;
			minSpeedXEdit = 0;
			maxSpeedYEdit = 0;
			minSpeedYEdit = 0;
			accelXGainEdit = 0;
			accelYGainEdit = 0;
			healthEdit = 0;
			rotatingEdit = 0;
			gravityEdit = 0;
			
			chaseOffsetLeftEdit = 0;
			chaseOffsetRightEdit = 0;
			chaseOffsetUpEdit = 0;
			chaseOffsetDownEdit = 0;

			movesStartingLeftBoxEdit = 0;
			movesStartingRightBoxEdit = 0;
			movesStartingUpBoxEdit = 0;
			movesStartingDownBoxEdit = 0;
		}
		
		if(showSaveMessage) {
			DrawRectangle(0, 0, AI_EDIT_WINDOW_WIDTH, AI_EDIT_WINDOW_HEIGHT, Fade(RAYWHITE, 0.8f));
			int btnActive = -1;
            GuiMessageBox((Rectangle){ (AI_EDIT_WINDOW_WIDTH/2)-300, (AI_EDIT_WINDOW_HEIGHT/2)-150, 650, 300 },
            	"#191# Exported", "AI List exported.\nRun the build script for the changes to take effect.", "Okay", &btnActive);

            if(btnActive >= 0) showSaveMessage = 0;
		}
		
		EndDrawing();
	}

	UnloadTexture(AIStateTab);
	UnloadTexture(AITab);
	UnloadTexture(AIStateTabFocused);
	UnloadTexture(AITabFocused);

	CloseWindow();
	return 0;
}