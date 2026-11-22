#include <raylib.h>
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "level.h"

#define BG_EDIT 1
#define MUSIC_EDIT 2
#define TILESET1_EDIT 3
#define TILESET2_EDIT 4

#define LEVEL_EDIT_WINDOW_WIDTH (LEVEL_WIDTH_PIXEL+1000)
#define LEVEL_EDIT_WINDOW_HEIGHT (LEVEL_HEIGHT_PIXEL)

#define LEVEL_EDIT_MESSAGE_BOX_START_X ((LEVEL_EDIT_WINDOW_WIDTH)-1450)
#define LEVEL_EDIT_MESSAGE_BOX_START_Y ((LEVEL_EDIT_WINDOW_HEIGHT)-450)

#define LEVEL_EDIT_MESSAGE_BOX_WIDTH (400)
#define LEVEL_EDIT_MESSAGE_BOX_HEIGHT (LEVEL_EDIT_WINDOW_HEIGHT-100)

#define LEVEL_EDIT_MESSAGE_BOX_END_X (LEVEL_EDIT_MESSAGE_BOX_START_X + LEVEL_EDIT_MESSAGE_BOX_WIDTH)
#define LEVEL_EDIT_MESSAGE_BOX_END_Y (LEVEL_EDIT_MESSAGE_BOX_START_Y + LEVEL_EDIT_MESSAGE_BOX_HEIGHT)

#define LEVEL_EDIT_MESSAGE_BOX_OFFSET_X (5)
#define LEVEL_EDIT_MESSAGE_BOX_OFFSET_Y (25)


int solidMode = 0;
int solidOTMode = 0;
int solidOLMode = 0;
int solidORMode = 0;
int solidOBMode = 0;
int jumpBoostMode = 0;
int harmfulMode = 0;
int instaDeathMode = 0;
int showGrid = 1;

int selectedTile = 0;
int selectedTexture = 0;
int selectedAI = 0;
int selectedSprite = 0;

int texWidth = 16;
int texHeight = 16;
int spriteWidth = 16;
int spriteHeight = 16;
int spriteCount = 0;

int spriteWidthEdit = 0;
int spriteHeightEdit = 0;
int texWidthEdit = 0;
int texHeightEdit = 0;

int levelMusicEdit = 0;
int levelBGEdit = 0;
int levelTilesetEdit = 0;

int tileEditMode = 1;
int showSprites = 1;

int showBackground = 1;

int tileset1Changed = 0;
int backgroundChanged = 0;

int spawnX = 0;
int spawnY = 0;

int noEdit = 0;
int noEditBG = 0;
int noEditMusic = 0;
int noEditTileset = 0;

AIState UnusedAIState = {0};
SpriteAI UnusedAI = {{0}, 0};

Texture2D NoTexture = {0};
//FinalSprite UnusedSprite = {UnusedAI, NoTexture, 16, 16, 0, 0, 1};

void DrawLevel(Texture2D tileset, Texture2D symbols) {
	for(int y = 0; y < LEVEL_HEIGHT; y++) {
		for(int x = 0; x < LEVEL_WIDTH; x++) {
			int sprite = level[y][x].sprite;

			if(sprite != -1 && sprite != 255) {
				DrawTextureRec(
					tileset,
					(Rectangle){
						sprite * 16,
						0,
						16,
						16
					},
					(Vector2){
						x * 16,
						y * 16
					},
					WHITE
				);
			}

			else if(level[y][x].sprite == -1 && !showBackground)
				DrawRectangle(x*16, y*16, 16, 16, BACKGROUND_COLOR_GLOBAL);

			if(showGrid)
				DrawRectangleLines(x*16, y*16, 16, 16, (Color){255, 255, 255, 40});

			if(level[y][x].solidOT)
				DrawTextureRec(
					symbols,
					(Rectangle){0, 0, 16, 16},
					(Vector2){x*16, y*16},
					WHITE);

			if(level[y][x].solidOL)
				DrawTextureRec(
					symbols,
					(Rectangle){16, 0, 16, 16},
					(Vector2){x*16, y*16},
					WHITE);

			if(level[y][x].solidOR)
				DrawTextureRec(
					symbols,
					(Rectangle){32, 0, 16, 16},
					(Vector2){x*16, y*16},
					WHITE);

			if(level[y][x].solidOB)
				DrawTextureRec(
					symbols,
					(Rectangle){48, 0, 16, 16},
					(Vector2){x*16, y*16},
					WHITE);
			if(level[y][x].jumpBoost)
				DrawTextureRec(
					symbols,
					(Rectangle){64, 0, 16, 16},
					(Vector2){x*16, y*16},
					WHITE);
			if(level[y][x].harmful)
				DrawTextureRec(
					symbols,
					(Rectangle){80, 0, 16, 16},
					(Vector2){x*16, y*16},
					WHITE);
			if(level[y][x].instaDeath)
				DrawTextureRec(
					symbols,
					(Rectangle){96, 0, 16, 16},
					(Vector2){x*16, y*16},
					WHITE);

			if(level[y][x].sprite == 255)
				DrawRectangleLines(x*16, y*16, 16, 16, GREEN);
		}
	}
}

void DrawTileset(Texture2D tileset) {
	//for(int i = 0; i < 4; i++) {}
	for(int x = 0; x < 16; x+=2)
		DrawTexturePro(
			tileset,
			(Rectangle){(x/2)*16, 0, 16, 16},
			(Rectangle){(x+33)*16, 15*16, 32, 32},
			(Vector2){0,0},
			0.0f, WHITE);
	for(int x = 16; x < 32; x+=2)
		DrawTexturePro(
			tileset,
			(Rectangle){(x/2)*16, 0, 16, 16},
			(Rectangle){(x+17)*16, 17*16, 32, 32},
			(Vector2){0,0},
			0.0f, WHITE);
	for(int x = 32; x < 48; x+=2)
		DrawTexturePro(
			tileset,
			(Rectangle){(x/2)*16, 0, 16, 16},
			(Rectangle){(x+1)*16, 19*16, 32, 32},
			(Vector2){0,0},
			0.0f, WHITE);
	for(int x = 48; x < 60; x+=2)
		DrawTexturePro(
			tileset,
			(Rectangle){(x/2)*16, 0, 16, 16},
			(Rectangle){(x-15)*16, 21*16, 32, 32},
			(Vector2){0,0},
			0.0f, WHITE);
}

void EditTile(Vector2 mouse) {
	if(mouse.x >= 0 &&
	   mouse.y >= 0 &&
	   mouse.x < LEVEL_WIDTH * 16 &&
	   mouse.y < LEVEL_HEIGHT * 16) {
		int tx = (int)mouse.x / 16;
		int ty = (int)mouse.y / 16;
		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			level[ty][tx].sprite = selectedTile;
			level[ty][tx].solidOT = solidOTMode;
			level[ty][tx].solidOL = solidOLMode;
			level[ty][tx].solidOR = solidORMode;
			level[ty][tx].solidOB = solidOBMode;
			level[ty][tx].jumpBoost = jumpBoostMode;
			level[ty][tx].harmful = harmfulMode;
			level[ty][tx].instaDeath = instaDeathMode;
		}
		else if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
			level[ty][tx].sprite = -1; level[ty][tx].solidOT = 0; level[ty][tx].solidOL = 0; level[ty][tx].solidOR = 0; level[ty][tx].solidOB = 0; level[ty][tx].jumpBoost = 0; level[ty][tx].harmful = 0; level[ty][tx].instaDeath = 0;}

		else if(IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
			level[ty][tx].sprite = 255; level[ty][tx].solidOT = 0; level[ty][tx].solidOL = 0; level[ty][tx].solidOR = 0; level[ty][tx].solidOB = 0; level[ty][tx].jumpBoost = 0; level[ty][tx].harmful = 0; level[ty][tx].instaDeath = 0;}
	}
}

void EditSprite(Vector2 mouse, Texture2D *textures, char **textureNames, int textureCount) {
	if(showSprites) {
		if(mouse.x >= 0 &&
		mouse.y >= 0 &&
		mouse.x < ((LEVEL_WIDTH * 16)-8) &&
		mouse.y < ((LEVEL_HEIGHT * 16)-8)) {
			int tx = (int)mouse.x / 8;
			int ty = (int)mouse.y / 8;
			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && spriteCount < SPRITE_LIMIT) {
				selectedSprite = -1;
				for(int i = 0; i < SPRITE_LIMIT; i++) {
					if(sprites[i].unused) {selectedSprite = i; break;}
				}
				if(selectedSprite >= 0 && selectedSprite < SPRITE_LIMIT) {
					sprites[selectedSprite].unused = 0;
					strcpy(sprites[selectedSprite].AIName, AINames[selectedAI]);
					strcpy(sprites[selectedSprite].textureName, textureNames[selectedTexture]);
					for(int i = 0; i < SPRITE_LIMIT; i++) {
						for(int j = 0; j < textureCount; j++) {
							if(strcmp(sprites[i].textureName, textureNames[j]) == 0) {
								sprites[i].textureID = j;
							}
						}

						/*for(int j = 0; j < AI_LIMIT; j++) {
							if(strcmp(sprites[i].AIName, AINames[j]) == 0) {
								sprites[i].AI_ID = j; // ??? whoopsie
							}
						}*/
					}
					sprites[selectedSprite].width = spriteWidth;
					sprites[selectedSprite].height = spriteHeight;
					sprites[selectedSprite].texWidth = texWidth;
					sprites[selectedSprite].texHeight = texHeight;
					sprites[selectedSprite].x = tx*8;
					sprites[selectedSprite].y = ty*8;
					spriteCount++;
				}
			}
			if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				for(int i = 0; i < SPRITE_LIMIT; i++) {
					Rectangle spriteArea = {sprites[i].x, sprites[i].y, sprites[i].width, sprites[i].height};
					if(CheckCollisionPointRec(mouse, spriteArea) && !sprites[i].unused) {
						sprites[i].unused = 1;
						if(spriteCount > 0) spriteCount--;
						break;
					}
				}
			}
		}
	}
}

void ShowMessageBox(Vector2 mouse, char **bgNames, char **musicNames, char **tilesetNames, int bgCount, int musicCount, int tilesetCount, int mode) {
	if(mode == BG_EDIT) {
		if(levelBGEdit) {
			DrawRectangle(0, 0, LEVEL_EDIT_WINDOW_WIDTH, LEVEL_EDIT_WINDOW_HEIGHT, Fade(RAYWHITE, 0.8f));
			int btnActive = -1;
			GuiMessageBox((Rectangle){
				LEVEL_EDIT_MESSAGE_BOX_START_X, LEVEL_EDIT_MESSAGE_BOX_START_Y, LEVEL_EDIT_MESSAGE_BOX_WIDTH, LEVEL_EDIT_MESSAGE_BOX_HEIGHT },
				"#012# Select a Background", "", "Done", &btnActive);


			int bgFontSize = (LEVEL_EDIT_MESSAGE_BOX_HEIGHT) / (bgCount == 0 ? 1 : bgCount);
			if(bgFontSize > 20) bgFontSize = 20;

			for(int i = 0; i < bgCount; i++) {
				Color textColor = strcmp(bgNames[i], levelBGText) ? BLACK : BLUE;
				DrawText(bgNames[i], LEVEL_EDIT_MESSAGE_BOX_START_X+LEVEL_EDIT_MESSAGE_BOX_OFFSET_X,
						 (bgFontSize*(i))+LEVEL_EDIT_MESSAGE_BOX_START_Y+LEVEL_EDIT_MESSAGE_BOX_OFFSET_Y,
						 bgFontSize, textColor);
			}

			// outline and select
			for(int y = 0; y < bgCount; y++) {
				int rowY = (LEVEL_EDIT_MESSAGE_BOX_START_Y+LEVEL_EDIT_MESSAGE_BOX_OFFSET_Y) + (y*bgFontSize);
				if(mouse.x >= LEVEL_EDIT_MESSAGE_BOX_START_X+LEVEL_EDIT_MESSAGE_BOX_OFFSET_X &&
				mouse.x < LEVEL_EDIT_MESSAGE_BOX_END_X-LEVEL_EDIT_MESSAGE_BOX_OFFSET_X &&
				mouse.y >= rowY &&
				mouse.y < rowY + bgFontSize) {
					DrawRectangleLines(LEVEL_EDIT_MESSAGE_BOX_START_X+(LEVEL_EDIT_MESSAGE_BOX_OFFSET_X-3), rowY,
									   LEVEL_EDIT_MESSAGE_BOX_WIDTH-4, bgFontSize, BLACK);
					if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) strcpy(levelBGText, bgNames[y]);
				}
			}
			
			// close message box if pressed done or x
			if(btnActive == 0 || btnActive == 1) {
				levelBGEdit = 0;
				noEdit = 1;
				backgroundChanged = 1;
			}
		}
	}
	else if(mode == MUSIC_EDIT) {
		if(levelMusicEdit) {
			DrawRectangle(0, 0, LEVEL_EDIT_WINDOW_WIDTH, LEVEL_EDIT_WINDOW_HEIGHT, Fade(RAYWHITE, 0.8f));
			int btnActive = -1;
			GuiMessageBox((Rectangle){
				LEVEL_EDIT_MESSAGE_BOX_START_X, LEVEL_EDIT_MESSAGE_BOX_START_Y, LEVEL_EDIT_MESSAGE_BOX_WIDTH, LEVEL_EDIT_MESSAGE_BOX_HEIGHT },
				"#011# Select a Music", "", "Done", &btnActive);


			int musicFontSize = (LEVEL_EDIT_MESSAGE_BOX_HEIGHT) / (musicCount == 0 ? 1 : musicCount);
			if(musicFontSize > 20) musicFontSize = 20;

			for(int i = 0; i < musicCount; i++) {
				Color textColor = strcmp(musicNames[i], levelMusicText) ? BLACK : BLUE;
				DrawText(musicNames[i], LEVEL_EDIT_MESSAGE_BOX_START_X+LEVEL_EDIT_MESSAGE_BOX_OFFSET_X,
						 (musicFontSize*(i))+LEVEL_EDIT_MESSAGE_BOX_START_Y+LEVEL_EDIT_MESSAGE_BOX_OFFSET_Y,
						 musicFontSize, textColor);
			}

			// outline and select
			for(int y = 0; y < musicCount; y++) {
				int rowY = (LEVEL_EDIT_MESSAGE_BOX_START_Y+LEVEL_EDIT_MESSAGE_BOX_OFFSET_Y) + (y*musicFontSize);
				if(mouse.x >= LEVEL_EDIT_MESSAGE_BOX_START_X+LEVEL_EDIT_MESSAGE_BOX_OFFSET_X &&
				mouse.x < LEVEL_EDIT_MESSAGE_BOX_END_X-LEVEL_EDIT_MESSAGE_BOX_OFFSET_X &&
				mouse.y >= rowY &&
				mouse.y < rowY + musicFontSize) {
					DrawRectangleLines(LEVEL_EDIT_MESSAGE_BOX_START_X+(LEVEL_EDIT_MESSAGE_BOX_OFFSET_X-3), rowY,
									   LEVEL_EDIT_MESSAGE_BOX_WIDTH-4, musicFontSize, BLACK);
					if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) strcpy(levelMusicText, musicNames[y]);
				}
			}
			
			// close message box if pressed done or x
			if(btnActive == 0 || btnActive == 1) {
				levelMusicEdit = 0;
				noEdit = 1;
			}
		}
	}
	else if(mode == TILESET1_EDIT) {
		if(levelTilesetEdit) {
			DrawRectangle(0, 0, LEVEL_EDIT_WINDOW_WIDTH, LEVEL_EDIT_WINDOW_HEIGHT, Fade(RAYWHITE, 0.8f));
			int btnActive = -1;
			GuiMessageBox((Rectangle){
				LEVEL_EDIT_MESSAGE_BOX_START_X, LEVEL_EDIT_MESSAGE_BOX_START_Y, LEVEL_EDIT_MESSAGE_BOX_WIDTH, LEVEL_EDIT_MESSAGE_BOX_HEIGHT },
				"#012# Select a Tileset (1)", "", "Done", &btnActive);


			int tilesetFontSize = (LEVEL_EDIT_MESSAGE_BOX_HEIGHT) / (tilesetCount == 0 ? 1 : tilesetCount);
			if(tilesetFontSize > 20) tilesetFontSize = 20;

			for(int i = 0; i < tilesetCount; i++) {
				Color textColor = strcmp(tilesetNames[i], levelTilesetText) ? BLACK : BLUE;
				DrawText(tilesetNames[i], LEVEL_EDIT_MESSAGE_BOX_START_X+LEVEL_EDIT_MESSAGE_BOX_OFFSET_X,
						 (tilesetFontSize*(i))+LEVEL_EDIT_MESSAGE_BOX_START_Y+LEVEL_EDIT_MESSAGE_BOX_OFFSET_Y,
						 tilesetFontSize, textColor);
			}

			// outline and select
			for(int y = 0; y < tilesetCount; y++) {
				int rowY = (LEVEL_EDIT_MESSAGE_BOX_START_Y+LEVEL_EDIT_MESSAGE_BOX_OFFSET_Y) + (y*tilesetFontSize);
				if(mouse.x >= LEVEL_EDIT_MESSAGE_BOX_START_X+LEVEL_EDIT_MESSAGE_BOX_OFFSET_X &&
				mouse.x < LEVEL_EDIT_MESSAGE_BOX_END_X-LEVEL_EDIT_MESSAGE_BOX_OFFSET_X &&
				mouse.y >= rowY &&
				mouse.y < rowY + tilesetFontSize) {
					DrawRectangleLines(LEVEL_EDIT_MESSAGE_BOX_START_X+(LEVEL_EDIT_MESSAGE_BOX_OFFSET_X-3), rowY,
									   LEVEL_EDIT_MESSAGE_BOX_WIDTH-4, tilesetFontSize, BLACK);
					if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) strcpy(levelTilesetText, tilesetNames[y]);
				}
			}
			
			// close message box if pressed done or x
			if(btnActive == 0 || btnActive == 1) {
				levelTilesetEdit = 0;
				noEdit = 1;
				tileset1Changed = 1;
			}
		}
	}
	
}

void EditOther(Vector2 mouse, Texture2D *textures, char **textureNames, int textureCount) {
	Rectangle levelMusicBox = {LEVEL_WIDTH_PIXEL+380, 8, 300, 20};
	Rectangle levelBGBox = {LEVEL_WIDTH_PIXEL+380, 29, 300, 20};
	Rectangle levelTilesetBox = {LEVEL_WIDTH_PIXEL+380, 50, 300, 20};
	Rectangle spriteWidthBox = {LEVEL_WIDTH_PIXEL+580, 80, 100, 20};
	Rectangle spriteHeightBox = {LEVEL_WIDTH_PIXEL+580, 100, 100, 20};
	Rectangle texWidthBox = {LEVEL_WIDTH_PIXEL+580, 120, 100, 20};
	Rectangle texHeightBox = {LEVEL_WIDTH_PIXEL+580, 140, 100, 20};

	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		spriteWidthEdit = CheckCollisionPointRec(mouse, spriteWidthBox);
		spriteHeightEdit = CheckCollisionPointRec(mouse, spriteHeightBox);
		texWidthEdit = CheckCollisionPointRec(mouse, texWidthBox);
		texHeightEdit = CheckCollisionPointRec(mouse, texHeightBox);
		levelMusicEdit = CheckCollisionPointRec(mouse, levelMusicBox);
		levelBGEdit = CheckCollisionPointRec(mouse, levelBGBox);
		levelTilesetEdit = CheckCollisionPointRec(mouse, levelTilesetBox);
	}

	
	// if(noEditMusic)
		if(GuiButton(levelMusicBox, "#011# Level Music")) levelMusicEdit = 1;
		//GuiTextBox(levelMusicBox, levelMusicText, sizeof(levelMusicText), levelMusicEdit);
	// if(noEditBG)
		if(GuiButton(levelBGBox, "#012# Level Background")) levelBGEdit = 1;
		//GuiTextBox(levelBGBox, levelBGText, sizeof(levelBGText), levelBGEdit);
	// if(noEditTileset) 
		if(GuiButton(levelTilesetBox, "#012# Level Tileset")) levelTilesetEdit = 1;
		//GuiTextBox(levelTilesetBox, levelTilesetText, sizeof(levelTilesetText), levelTilesetEdit);

	DrawText("Sprite Width (pixels):", LEVEL_WIDTH_PIXEL+340, 80, 20, BLACK);
	GuiValueBox(spriteWidthBox, NULL, &spriteWidth, 1, LEVEL_WIDTH_PIXEL, spriteWidthEdit);
	DrawText("Sprite Height (pixels):", LEVEL_WIDTH_PIXEL+340, 100, 20, BLACK);
	GuiValueBox(spriteHeightBox, NULL, &spriteHeight, 1, LEVEL_HEIGHT_PIXEL, spriteHeightEdit);

	DrawText("Texture Width (pixels):", LEVEL_WIDTH_PIXEL+340, 120, 20, BLACK);
	GuiValueBox(texWidthBox, NULL, &texWidth, 1, LEVEL_WIDTH_PIXEL, texWidthEdit);
	DrawText("Texture Height (pixels):", LEVEL_WIDTH_PIXEL+340, 140, 20, BLACK);
	GuiValueBox(texHeightBox, NULL, &texHeight, 1, LEVEL_HEIGHT_PIXEL, texHeightEdit);

	DrawText("Sprite Textures:", LEVEL_WIDTH_PIXEL+330, 170, 20, BLACK);
	DrawRectangleLines(LEVEL_WIDTH_PIXEL+330, 200, 300, LEVEL_HEIGHT_PIXEL-210, BLACK);
	DrawText("Sprite AIs:", LEVEL_WIDTH_PIXEL+660, 170, 20, BLACK);
	DrawRectangleLines(LEVEL_WIDTH_PIXEL+660, 200, 300, LEVEL_HEIGHT_PIXEL-210, BLACK);

	int textureFontSize = (LEVEL_HEIGHT_PIXEL-220) / (textureCount == 0 ? 1 : textureCount);
	if(textureFontSize > 20) textureFontSize = 20;
	int AIFontSize = (LEVEL_HEIGHT_PIXEL-220)/AI_LIMIT;
	if(AIFontSize > 20) AIFontSize = 20;
	for(int i = 0; i < textureCount; i++) {
		DrawText(textureNames[i], LEVEL_WIDTH_PIXEL+340, (textureFontSize*(i))+210, textureFontSize, selectedTexture == i ? BLUE : BLACK);
	}
	for(int i = 0; i < AI_LIMIT; i++) {
		Color textColor = strcmp(AINames[i], UNUSED_TEXT) ? BLACK : LIGHTGRAY;
		DrawText(AINames[i], LEVEL_WIDTH_PIXEL+670, (AIFontSize*(i))+210, AIFontSize, selectedAI == i ? BLUE : textColor);
	}

	if(noEdit) {
		// outline and select
		for(int y = 0; y < textureCount; y++) {
			int rowY = 210 + (y*textureFontSize);
			if(mouse.x >= LEVEL_WIDTH_PIXEL+335 &&
			mouse.x < LEVEL_WIDTH_PIXEL+625 &&
			mouse.y >= rowY &&
			mouse.y < rowY + textureFontSize) {
				DrawRectangleLines(LEVEL_WIDTH_PIXEL+335, rowY, 290, textureFontSize, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) selectedTexture = y;
			}
		}
		for(int y = 0; y < AI_LIMIT; y++) {
			int rowY = 210 + (y*AIFontSize);
			if(mouse.x >= LEVEL_WIDTH_PIXEL+665 &&
			mouse.x < LEVEL_WIDTH_PIXEL+955 &&
			mouse.y >= rowY &&
			mouse.y < rowY + AIFontSize) {
				DrawRectangleLines(LEVEL_WIDTH_PIXEL+665, rowY, 290, AIFontSize, BLACK);
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) selectedAI = y;
			}
		}
	}
}

void ExportLevelC(void) {
	FILE *level_export = fopen("level.c", "w");

	if(!level_export) {printf("ERROR: level file not found?"); return;}

	fprintf(level_export, "#include \"level.h\"\n\nstruct Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {\n");
	for(int y = 0; y < LEVEL_HEIGHT-1; y++) {
		fprintf(level_export, "\t{");
		for(int x = 0; x < LEVEL_WIDTH-1; x++)
			fprintf(level_export, "{%d, %d, %d, %d, %d, %d, %d, %d}, ", level[y][x].sprite, level[y][x].solidOT, level[y][x].solidOL, level[y][x].solidOR, level[y][x].solidOB, level[y][x].jumpBoost, level[y][x].harmful, level[y][x].instaDeath);
		fprintf(level_export, "{%d, %d, %d, %d, %d, %d, %d, %d}},\n", level[y][LEVEL_WIDTH-1].sprite, level[y][LEVEL_WIDTH-1].solidOT, level[y][LEVEL_WIDTH-1].solidOL, level[y][LEVEL_WIDTH-1].solidOR, level[y][LEVEL_WIDTH-1].solidOB, level[y][LEVEL_WIDTH-1].jumpBoost, level[y][LEVEL_WIDTH-1].harmful, level[y][LEVEL_WIDTH-1].instaDeath);
	}
	fprintf(level_export, "\t{");
	for(int x = 0; x < LEVEL_WIDTH-1; x++)
		fprintf(level_export, "{%d, %d, %d, %d, %d, %d, %d, %d}, ", level[LEVEL_HEIGHT-1][x].sprite, level[LEVEL_HEIGHT-1][x].solidOT, level[LEVEL_HEIGHT-1][x].solidOL, level[LEVEL_HEIGHT-1][x].solidOR, level[LEVEL_HEIGHT-1][x].solidOB, level[LEVEL_HEIGHT-1][x].jumpBoost, level[LEVEL_HEIGHT-1][x].harmful, level[LEVEL_HEIGHT-1][x].instaDeath);
	fprintf(level_export, "{%d, %d, %d, %d, %d, %d, %d, %d}}\n};\n\n", level[LEVEL_HEIGHT-1][LEVEL_WIDTH-1].sprite, level[LEVEL_HEIGHT-1][LEVEL_WIDTH-1].solidOT, level[LEVEL_HEIGHT-1][LEVEL_WIDTH-1].solidOL, level[LEVEL_HEIGHT-1][LEVEL_WIDTH-1].solidOR, level[LEVEL_HEIGHT-1][LEVEL_WIDTH-1].solidOB, level[LEVEL_HEIGHT-1][LEVEL_WIDTH-1].jumpBoost, level[LEVEL_HEIGHT-1][LEVEL_WIDTH-1].harmful, level[LEVEL_HEIGHT-1][LEVEL_WIDTH-1].instaDeath);

	fprintf(level_export, "FinalSprite sprites[SPRITE_LIMIT] = {\n");
	for(int i = 0; i < SPRITE_LIMIT; i++) {
		fprintf(level_export, "{");
		// FORMAT
		//fprintf(level_export, "{0}, 0, 0, 16, 16, 0, 0, .unused = 1, 16, 16, 0}");
		fprintf(level_export, "{0}, \"%s\", %d, %f, %f, %f, %f, %d, %f, %f, \"%s\"}", sprites[i].AIName, sprites[i].textureID, sprites[i].width, sprites[i].height, sprites[i].x, sprites[i].y, sprites[i].unused, sprites[i].texWidth, sprites[i].texHeight, sprites[i].textureName);
		fprintf(level_export, "%s", i + 1 == SPRITE_LIMIT ? "\n" : ",\n\t");
	}
	fprintf(level_export, "};\n\n");

	// FORMAT
	//fprintf(level_export, "char levelMusicText[MAX_TEXT_CHAR] = \"\\0\";\nchar levelBGText[MAX_TEXT_CHAR] = \"\\0\";\nchar levelTilesetText[MAX_TEXT_CHAR] = \"\\0\";");
	fprintf(level_export, "char levelMusicText[MAX_TEXT_CHAR] = \"%s\";\nchar levelBGText[MAX_TEXT_CHAR] = \"%s\";\nchar levelTilesetText[MAX_TEXT_CHAR] = \"%s\";", levelMusicText, levelBGText, levelTilesetText);

	fclose(level_export);
};

int main(void) {
	InitWindow(LEVEL_EDIT_WINDOW_WIDTH, LEVEL_EDIT_WINDOW_HEIGHT, "Super Hydric Guy - Level Editor");

	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

	SetTargetFPS(60);

	Texture2D symbols = LoadTexture("gfx/level_editor/symbols.png");

	char levelBGTextModif[MAX_TEXT_CHAR+20] = {0};
	snprintf(levelBGTextModif, sizeof(levelBGTextModif), "gfx/bg/%s", levelBGText);
	Texture2D background = LoadTexture(levelBGTextModif);

	char levelTilesetTextModif[MAX_TEXT_CHAR+20] = {0};
	snprintf(levelTilesetTextModif, sizeof(levelTilesetTextModif), "gfx/tilesets/%s", levelTilesetText);
	Texture2D tileset1 = LoadTexture(levelTilesetTextModif);

	FilePathList spriteTextures = LoadDirectoryFiles("gfx/sprites");
	Texture2D *textures = MemAlloc(spriteTextures.count * sizeof(Texture2D));
	char **textureNames = MemAlloc(spriteTextures.count * sizeof(*textureNames));
	int textureCount = 0;

	FilePathList bgs = LoadDirectoryFiles("gfx/bg");
	char **bgNames = MemAlloc(bgs.count * sizeof(*bgNames));
	int bgCount = 0;

	FilePathList tilesets = LoadDirectoryFiles("gfx/tilesets");
	char **tilesetNames = MemAlloc(tilesets.count * sizeof(*tilesetNames));
	int tilesetCount = 0;

	FilePathList music = LoadDirectoryFiles("audio/music");
	char **musicNames = MemAlloc(music.count * sizeof(*musicNames));
	int musicCount = 0;

	// bg
	for(unsigned int i = 0; i < bgs.count; i++) {
		if(IsFileExtension(bgs.paths[i], ".png") ||
			IsFileExtension(bgs.paths[i], ".jpg")) {

			const char *name = GetFileName(bgs.paths[i]);
			char *nameCopy = malloc(strlen(name) + 1);

			if(nameCopy != NULL) {
				strcpy(nameCopy, name);
				bgNames[bgCount] = nameCopy;
				bgCount++;
			}
		}
	}
	// tileset
	for(unsigned int i = 0; i < tilesets.count; i++) {
		if(IsFileExtension(tilesets.paths[i], ".png") ||
			IsFileExtension(tilesets.paths[i], ".jpg")) {

			const char *name = GetFileName(tilesets.paths[i]);
			char *nameCopy = malloc(strlen(name) + 1);

			if(nameCopy != NULL) {
				strcpy(nameCopy, name);
				tilesetNames[tilesetCount] = nameCopy;
				tilesetCount++;
			}
		}
	}
	// music
	for(unsigned int i = 0; i < music.count; i++) {
			const char *name = GetFileName(music.paths[i]);
			char *nameCopy = malloc(strlen(name) + 1);

			if(nameCopy != NULL) {
				strcpy(nameCopy, name);
				musicNames[musicCount] = nameCopy;
				musicCount++;
			}
	}
	// sprites
	for(unsigned int i = 0; i < spriteTextures.count; i++) {
		if(IsFileExtension(spriteTextures.paths[i], ".png") ||
			IsFileExtension(spriteTextures.paths[i], ".jpg")) {

			/*textures[textureCount] = LoadTexture(spriteTextures.paths[i]);
			char *name = GetFileNameWithoutExt(spriteTextures.paths[i]);
			textureNames[textureCount] = malloc(strlen(name) + 1);

			if (textureNames[textureCount] != NULL) {
				strcpy(textureNames[textureCount], name);
				textureCount++;
			}*/

			const char *name = GetFileName(spriteTextures.paths[i]);
			char *nameCopy = malloc(strlen(name) + 1);

			if(nameCopy != NULL) {
				strcpy(nameCopy, name);
				textures[textureCount] = LoadTexture(spriteTextures.paths[i]);
				textureNames[textureCount] = nameCopy;
				textureCount++;
			}
		}
	}


	UnloadDirectoryFiles(spriteTextures);
	UnloadDirectoryFiles(music);
	UnloadDirectoryFiles(tilesets);
	UnloadDirectoryFiles(bgs);	

	char SpriteTileText[2][20] = {"Sprite", "Tile"};
	char HideShowText[2][20] = {"Hide", "Show"};

	for(int i = 0; i < SPRITE_LIMIT; i++) {
		if(!sprites[i].unused) {
			spriteCount++;
		}
	}


	int showSaveMessage = 0;

	while(!WindowShouldClose()) {
		if(tileset1Changed) {
			char levelTilesetTextModif[MAX_TEXT_CHAR+20] = {0};
			snprintf(levelTilesetTextModif, sizeof(levelTilesetTextModif), "gfx/tilesets/%s", levelTilesetText);
			tileset1 = LoadTexture(levelTilesetTextModif);

			tileset1Changed = 0;
		}

		if(backgroundChanged) {
			char levelBGTextModif[MAX_TEXT_CHAR+20] = {0};
			snprintf(levelBGTextModif, sizeof(levelBGTextModif), "gfx/bg/%s", levelBGText);
			background = LoadTexture(levelBGTextModif);

			backgroundChanged = 0;
		}
		
		noEdit = !texWidthEdit && !texHeightEdit && !spriteWidthEdit && !spriteHeightEdit && !levelBGEdit && !levelMusicEdit && !levelTilesetEdit && !showSaveMessage;
		noEditMusic = !texWidthEdit && !texHeightEdit && !spriteWidthEdit && !spriteHeightEdit && !levelBGEdit && !levelTilesetEdit && !showSaveMessage;
		noEditBG = !texWidthEdit && !texHeightEdit && !spriteWidthEdit && !spriteHeightEdit && !levelMusicEdit && !levelTilesetEdit && !showSaveMessage;
		noEditTileset = !texWidthEdit && !texHeightEdit && !spriteWidthEdit && !spriteHeightEdit && !levelMusicEdit && !levelBGEdit && !showSaveMessage;

		// keybinds
		if(noEdit) {
			if(IsKeyPressed(KEY_ONE)) {
				solidMode = !solidMode;
				solidOTMode = solidMode;
				solidOLMode = solidMode;
				solidORMode = solidMode;
				solidOBMode = solidMode;
			}

			if(IsKeyPressed(KEY_TWO))
				solidOTMode = !solidOTMode;

			if(IsKeyPressed(KEY_THREE))
				solidOLMode = !solidOLMode;

			if(IsKeyPressed(KEY_FOUR))
				solidORMode = !solidORMode;

			if(IsKeyPressed(KEY_FIVE))
				solidOBMode = !solidOBMode;

			if(IsKeyPressed(KEY_G))
				showGrid = !showGrid;

			if(IsKeyPressed(KEY_J))
				jumpBoostMode = !jumpBoostMode;

			if(IsKeyPressed(KEY_H))
				harmfulMode = !harmfulMode;
			
			if(IsKeyPressed(KEY_X))
				instaDeathMode = !instaDeathMode;

			if(IsKeyPressed(KEY_T))
				tileEditMode = !tileEditMode;

			if(IsKeyPressed(KEY_B))
				showBackground = !showBackground;

			if((!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S))
				showSprites = !showSprites;
		}
		
		if((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S)) {
			ExportLevelC();
			showSaveMessage = 1;
		}

		Vector2 mouse = GetMousePosition();

		if(noEdit) {

			for(int x = 0; x < 16; x+=2) {
				Rectangle rect = {(x+33)*16, 15*16, 32, 32};
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
				CheckCollisionPointRec(mouse, rect)) {
					selectedTile = x/2;
				}
			}
			for(int x = 16; x < 32; x+=2) {
				Rectangle rect = {(x+17)*16, 17*16, 32, 32};
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
				CheckCollisionPointRec(mouse, rect)) {
					selectedTile = x/2;
				}
			}
			for(int x = 32; x < 48; x+=2) {
				Rectangle rect = {(x+1)*16, 19*16, 32, 32};
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
				CheckCollisionPointRec(mouse, rect)) {
					selectedTile = x/2;
				}
			}
			for(int x = 48; x < 60; x+=2) {
				Rectangle rect = {(x-15)*16, 21*16, 32, 32};
				if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
				CheckCollisionPointRec(mouse, rect)) {
					selectedTile = x/2;
				}
			}
		}

		BeginDrawing();
		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

		// extra clear background color for where level is
		DrawRectangle(0, 0, LEVEL_WIDTH_PIXEL+16, LEVEL_HEIGHT_PIXEL, BACKGROUND_COLOR_GLOBAL);
		if(showBackground) DrawTextureRec(background, (Rectangle){0, 0, LEVEL_WIDTH_PIXEL, LEVEL_HEIGHT_PIXEL},
										  (Vector2){0,0}, WHITE);
		

		if(tileEditMode && noEdit) EditTile(mouse);

		DrawLevel(tileset1, symbols);
		if(!tileEditMode && noEdit) EditSprite(mouse, textures, textureNames, textureCount);
		DrawTileset(tileset1);
		EditOther(mouse, textures, textureNames, textureCount);
		
		Color solidColor = solidMode ? GREEN : GRAY;
		Color solidOTColor = solidOTMode ? GREEN : GRAY;
		Color solidOLColor = solidOLMode ? GREEN : GRAY;
		Color solidORColor = solidORMode ? GREEN : GRAY;
		Color solidOBColor = solidOBMode ? GREEN : GRAY;
		Color jumpBoostColor = jumpBoostMode ? BLUE : GRAY;
		Color harmfulColor = harmfulMode ? BLUE : GRAY;
		Color instaDeathColor = instaDeathMode ? BLUE : GRAY;
		Color showGridColor = !showGrid ? LIME : RED;
		Color showSpritesColor = !showSprites ? LIME : RED;

		DrawText(TextFormat("Selected Tile: %d", selectedTile), (LEVEL_WIDTH_PIXEL)+24, 8, 20, BLACK);
		DrawText("[Ctrl+S] Export Level (level.c)", (LEVEL_WIDTH_PIXEL)+24, 28, 20, BLACK);
		DrawText(TextFormat("[G] %s Grid", HideShowText[!showGrid]), (LEVEL_WIDTH_PIXEL)+24, 48, 20, showGridColor);
		DrawText("[1] Solid Mode", (LEVEL_WIDTH_PIXEL)+24, 68, 20, solidColor);
		DrawText("[2] Solid On Top Mode", (LEVEL_WIDTH_PIXEL)+24, 88, 20, solidOTColor);
		DrawText("[3] Solid On Left Mode", (LEVEL_WIDTH_PIXEL)+24, 108, 20, solidOLColor);
		DrawText("[4] Solid On Right Mode", (LEVEL_WIDTH_PIXEL)+24, 128, 20, solidORColor);
		DrawText("[5] Solid On Bottom Mode", (LEVEL_WIDTH_PIXEL)+24, 148, 20, solidOBColor);
		DrawText("[J] Jump Boost Powerup", (LEVEL_WIDTH_PIXEL)+24, 168, 20, jumpBoostColor);
		DrawText("[H] Harmful Mode", (LEVEL_WIDTH_PIXEL)+24, 188, 20, harmfulColor);
		DrawText("[X] Insta-Death Mode", (LEVEL_WIDTH_PIXEL)+24, 208, 20, instaDeathColor);

		DrawText(TextFormat("[LMB] Place Selected %s", SpriteTileText[tileEditMode]), (LEVEL_WIDTH_PIXEL)+24, 24*16, 20, BLACK);
		DrawText(TextFormat("[RMB] Remove %s", SpriteTileText[tileEditMode]), (LEVEL_WIDTH_PIXEL)+24, 25*16, 20, BLACK);
		if(tileEditMode) DrawText("[MMB] Place Spawn Tile", (LEVEL_WIDTH_PIXEL)+24, 26*16, 20, BLACK);
		DrawText(TextFormat("[T] Switch to %s Mode", SpriteTileText[!tileEditMode]), (LEVEL_WIDTH_PIXEL)+24, 27*16, 20, BLUE);
		DrawText(TextFormat("[S] %s Sprites", HideShowText[!showSprites]), (LEVEL_WIDTH_PIXEL)+24, 28*16, 20, showSpritesColor);
		DrawText(TextFormat("Sprite Count: %d", spriteCount), (LEVEL_WIDTH_PIXEL)+24, 29*16, 20, BLACK);
		DrawText(TextFormat("Selected Sprite: %d", selectedSprite), (LEVEL_WIDTH_PIXEL)+24, 30*16, 20, BLACK);
		DrawText(TextFormat("[B] %s Background", HideShowText[!showBackground]), LEVEL_WIDTH_PIXEL+700, 8, 20, showBackground ? RED : LIME);

		if(showSprites) {
			for(int i = 0; i < SPRITE_LIMIT; i++) {
				if(!sprites[i].unused) {
					DrawTexturePro(textures[sprites[i].textureID],
						(Rectangle){0, 0, sprites[i].texWidth, sprites[i].texHeight},
						(Rectangle){sprites[i].x, sprites[i].y, sprites[i].width, sprites[i].height},
						(Vector2){0, 0}, 0.0f, WHITE);
				}
			}
		}

		// ========== DEBUG ============
		//DrawText(TextFormat("Mouse X: %f", mouse.x), (LEVEL_WIDTH_PIXEL)+400, 20, 20, RED);
		//DrawText(TextFormat("Mouse Y: %f", mouse.y), (LEVEL_WIDTH_PIXEL)+400, 50, 20, RED);

		// white level outline
		DrawRectangleLines(
			0,
			0,
			LEVEL_WIDTH * 16,
			LEVEL_HEIGHT * 16,
			WHITE
		);

		if(noEdit) {
			// yellow level tile highlight
			if(tileEditMode) {
				if(mouse.x >= 0 &&
				mouse.y >= 0 &&
				mouse.x < LEVEL_WIDTH * 16 &&
				mouse.y < LEVEL_HEIGHT * 16) {
					int tx = (int)mouse.x / 16;
					int ty = (int)mouse.y / 16;

					DrawRectangleLines(
						tx * 16,
						ty * 16,
						16,
						16,
						YELLOW
					);
				}
			}
			// pink level sprite highlight
			else if(!tileEditMode) {
				if(mouse.x >= 0 &&
				mouse.y >= 0 &&
				mouse.x < ((LEVEL_WIDTH * 16)-8) &&
				mouse.y < ((LEVEL_HEIGHT * 16)-8)) {
					int tx = (int)mouse.x / 8;
					int ty = (int)mouse.y / 8;

					DrawRectangleLines(
						tx * 8,
						ty * 8,
						spriteWidth,
						spriteHeight,
						PINK
					);
				}
			}

			// black tileset tile outline
			if(mouse.x >= (LEVEL_WIDTH+1)*16 &&
			mouse.y >= 15*16 &&
			mouse.x < (LEVEL_WIDTH+1)*16 + 8*32 &&
			mouse.y < 15*16 + 4*32) {
				int tx = ((int)mouse.x - (LEVEL_WIDTH+1)*16) / 32;
				int ty = ((int)mouse.y - 15*16) / 32;
				DrawRectangleLines(
					(LEVEL_WIDTH+1)*16 + tx*32,
					15*16 + ty*32,
					32, 32, BLACK);}
		}
		
		if(showSaveMessage) {
			levelBGEdit = 0;
			levelMusicEdit = 0;
			levelTilesetEdit = 0;
		}
		
		ShowMessageBox(mouse, bgNames, musicNames, tilesetNames, bgCount, musicCount, tilesetCount, BG_EDIT);
		ShowMessageBox(mouse, bgNames, musicNames, tilesetNames, bgCount, musicCount, tilesetCount, MUSIC_EDIT);
		ShowMessageBox(mouse, bgNames, musicNames, tilesetNames, bgCount, musicCount, tilesetCount, TILESET1_EDIT);

		if(showSaveMessage) {
			DrawRectangle(0, 0, LEVEL_EDIT_WINDOW_WIDTH, LEVEL_EDIT_WINDOW_HEIGHT, Fade(RAYWHITE, 0.8f));
			int btnActive = -1;
            GuiMessageBox((Rectangle){ (LEVEL_EDIT_WINDOW_WIDTH/2)-300, (LEVEL_EDIT_WINDOW_HEIGHT/2)-150, 650, 300 },
            	"#191# Exported", "Level exported.\nRun the build script for the changes to take effect.", "Okay", &btnActive);

            if(btnActive >= 0) showSaveMessage = 0;
		}

		EndDrawing();
	}

	for(int i = 0; i < textureCount; i++) {
		free(textureNames[i]);
		UnloadTexture(textures[i]);
	}
	for(int i = 0; i < bgCount; i++) {
		free(bgNames[i]);
	}
	for(int i = 0; i < musicCount; i++) {
		free(musicNames[i]);
	}
	for(int i = 0; i < tilesetCount; i++) {
		free(tilesetNames[i]);
	}

	MemFree(tilesetNames);
	MemFree(musicNames);
	MemFree(bgNames);
	MemFree(textures);
	MemFree(textureNames);


	UnloadTexture(tileset1);
	UnloadTexture(background);
	UnloadTexture(symbols);

	CloseWindow();
	return 0;
}