#include <raylib.h>
#include <stdio.h>
#include "level.h"

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

int spawnX = -1;
int spawnY = -1;

void DrawLevel(Texture2D tileset, Texture2D symbols)
{
	for(int y = 0; y < LEVEL_HEIGHT; y++)
	{
		for(int x = 0; x < LEVEL_WIDTH; x++)
		{
			int sprite = level[y][x].sprite;

			if(sprite != -1 && sprite != 255)
			{
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

			else if(level[y][x].sprite == -1)
				DrawRectangle(x*16, y*16, 16, 16, BLACK);

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

void ExportLevelC(void) {
	FILE *level_export = fopen("level.c", "w");

	fprintf(level_export, "#include \"level.h\"\nstruct Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {\n");
	for(int y = 0; y < 31; y++) {
		fprintf(level_export, "\t{");
		for(int x = 0; x < 31; x++)
			fprintf(level_export, "{%d, %d, %d, %d, %d, %d, %d, %d}, ", level[y][x].sprite, level[y][x].solidOT, level[y][x].solidOL, level[y][x].solidOR, level[y][x].solidOB, level[y][x].jumpBoost, level[y][x].harmful, level[y][x].instaDeath);
		fprintf(level_export, "{%d, %d, %d, %d, %d, %d, %d, %d}},\n", level[y][31].sprite, level[y][31].solidOT, level[y][31].solidOL, level[y][31].solidOR, level[y][31].solidOB, level[y][31].jumpBoost, level[y][31].harmful, level[y][31].instaDeath);
	}
	fprintf(level_export, "\t{");
	for(int x = 0; x < 31; x++)
		fprintf(level_export, "{%d, %d, %d, %d, %d, %d, %d, %d}, ", level[31][x].sprite, level[31][x].solidOT, level[31][x].solidOL, level[31][x].solidOR, level[31][x].solidOB, level[31][x].jumpBoost, level[31][x].harmful, level[31][x].instaDeath);
	fprintf(level_export, "{%d, %d, %d, %d, %d, %d, %d, %d}}\n};", level[31][31].sprite, level[31][31].solidOT, level[31][31].solidOL, level[31][31].solidOR, level[31][31].solidOB, level[31][31].jumpBoost, level[31][31].harmful, level[31][31].instaDeath);

	fclose(level_export);
};

int main(void) {
	InitWindow(800, 512, "Super Hydric Guy - Level Editor");
	SetTargetFPS(60);

	Texture2D tileset1 = LoadTexture("gfx/forest_tileset.png");
	Texture2D symbols = LoadTexture("gfx/symbols.png");

	while(!WindowShouldClose()) {
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

		if(IsKeyPressed(KEY_S))
			ExportLevelC();

		if(IsKeyPressed(KEY_J))
			jumpBoostMode = !jumpBoostMode;

		if(IsKeyPressed(KEY_H))
			harmfulMode = !harmfulMode;
		
		if(IsKeyPressed(KEY_X))
			instaDeathMode = !instaDeathMode;

		Vector2 mouse = GetMousePosition();

		for(int x = 0; x < 16; x+=2) {
			Rectangle rect = {(x+33)*16, 15*16, 32, 32};
			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
			CheckCollisionPointRec(mouse, rect))
				selectedTile = x/2;
		}
		for(int x = 16; x < 32; x+=2) {
			Rectangle rect = {(x+17)*16, 17*16, 32, 32};
			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
			CheckCollisionPointRec(mouse, rect))
				selectedTile = x/2;
		}
		for(int x = 32; x < 48; x+=2) {
			Rectangle rect = {(x+1)*16, 19*16, 32, 32};
			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
			CheckCollisionPointRec(mouse, rect))
				selectedTile = x/2;
		}
		for(int x = 48; x < 60; x+=2) {
			Rectangle rect = {(x-15)*16, 21*16, 32, 32};
			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
			CheckCollisionPointRec(mouse, rect))
				selectedTile = x/2;
		}

		EditTile(mouse);

		BeginDrawing();   
		ClearBackground(BLACK);

		DrawLevel(tileset1, symbols);

		DrawRectangle((LEVEL_WIDTH+1)*16, 0, 350, 512, WHITE);

		DrawTileset(tileset1);

		/*for(int y = 0; y < 3; y++)
			for(int x = 0; x < 8; x++)
				DrawRectangleLines(
					(x+1)*(LEVEL_WIDTH+4)+32,
					((y+1)*5)+32,
					32,
					32,
					BLACK);//(Color){255, 255, 255, 40});
		for(int x = 0; x < 6; x++)
				DrawRectangleLines(
					x*32,
					32,
					32,
					32,
					(Color){255, 255, 255, 40});*/

		Color solidColor = solidMode ? GREEN : GRAY;
		Color solidOTColor = solidOTMode ? GREEN : GRAY;
		Color solidOLColor = solidOLMode ? GREEN : GRAY;
		Color solidORColor = solidORMode ? GREEN : GRAY;
		Color solidOBColor = solidOBMode ? GREEN : GRAY;
		Color jumpBoostColor = jumpBoostMode ? BLUE : GRAY;
		Color harmfulColor = harmfulMode ? BLUE : GRAY;
		Color instaDeathColor = instaDeathMode ? BLUE : GRAY;
		

		DrawText(TextFormat("Selected Tile: %d", selectedTile), (LEVEL_WIDTH*16)+24, 8, 20, BLACK);
		DrawText("[S] Export Level (level.c)", (LEVEL_WIDTH*16)+24, 28, 20, BLACK);
		if(!showGrid) DrawText("[G] Show Grid", (LEVEL_WIDTH*16)+24, 48, 20, LIME);
		else DrawText("[G] Hide Grid", (LEVEL_WIDTH*16)+24, 48, 20, RED);
		DrawText("[1] Solid Mode", (LEVEL_WIDTH*16)+24, 68, 20, solidColor);
		DrawText("[2] Solid On Top Mode", (LEVEL_WIDTH*16)+24, 88, 20, solidOTColor);
		DrawText("[3] Solid On Left Mode", (LEVEL_WIDTH*16)+24, 108, 20, solidOLColor);
		DrawText("[4] Solid On Right Mode", (LEVEL_WIDTH*16)+24, 128, 20, solidORColor);
		DrawText("[5] Solid On Bottom Mode", (LEVEL_WIDTH*16)+24, 148, 20, solidOBColor);
		DrawText("[J] Jump Boost Powerup", (LEVEL_WIDTH*16)+24, 168, 20, jumpBoostColor);
		DrawText("[H] Harmful Mode", (LEVEL_WIDTH*16)+24, 188, 20, harmfulColor);
		DrawText("[X] Insta-Death Mode", (LEVEL_WIDTH*16)+24, 208, 20, instaDeathColor);

		DrawText("[LMB] Place Selected Tile", (LEVEL_WIDTH*16)+24, 24*16, 20, BLACK);
		DrawText("[RMB] Remove Tile", (LEVEL_WIDTH*16)+24, 25*16, 20, BLACK);
		DrawText("[MMB] Place Spawn Tile", (LEVEL_WIDTH*16)+24, 26*16, 20, BLACK);

		//DrawText(TextFormat("Mouse X: %f", mouse.x), 16, 620, 20, WHITE);
		//DrawText(TextFormat("Mouse Y: %f", mouse.y), 16, 640, 20, WHITE);

		DrawRectangleLines(
			0,
			0,
			LEVEL_WIDTH * 16,
			LEVEL_HEIGHT * 16,
			WHITE
		);

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

		EndDrawing();
	}

	UnloadTexture(symbols);
	UnloadTexture(tileset1);
	CloseWindow();
	return 0;
}