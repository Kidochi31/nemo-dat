/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#include "window.h"
#include "ui.h"
#include "nemosection.h"
#include <stdio.h>


int main ()
{
	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);

	WindowSetup("Nemo Dat", "wabbit_alpha.png");

	// Load a texture from the resources directory
	Texture wabbit = LoadTexture("wabbit_alpha.png");
	Texture line = LoadTexture("vert_line.png");

	SetExitKey(KEY_NULL);
	
	Font font = GetFontDefault();
	UIText* input_text = UICreateText((Rectangle){.x=100,.y=100,.width=500,.height=500}, 60, PURPLE, "TEST", 1.5f, 2, &font);

	SetText set_text;

	UIConvertTextToSetText(input_text, &set_text);


	size_t selected_position = 0;
	Vector2 cursor = SetTextGetPositionOfChar(&set_text, selected_position);
	// int x_cursor = input_text->rectangle.x + UIGetTextWidthWithLength(input_text, selected_position);
	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{

		// drawing
		BeginDrawing();

		char k;
		bool text_changed = false;
		while (k = GetCharPressed()) {
			input_text->text = string_insert(input_text->text, selected_position, k);
			selected_position += 1;
			//x_cursor = input_text->rectangle.x + UIGetTextWidthWithLength(input_text, selected_position);
			text_changed = true;
		}

		if ((IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) && selected_position != strlen(input_text->text)) {
			input_text->text = string_remove(input_text->text, selected_position);
			//x_cursor = input_text->rectangle.x + UIGetTextWidthWithLength(input_text, selected_position);
			text_changed = true;
		}

		if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && selected_position != 0) {
			input_text->text = string_remove(input_text->text, selected_position - 1);
			selected_position -= 1;
			//x_cursor = input_text->rectangle.x + UIGetTextWidthWithLength(input_text, selected_position);
			text_changed = true;
		}

		if ((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && selected_position != 0) {
			selected_position -= 1;
			//x_cursor = input_text->rectangle.x + UIGetTextWidthWithLength(input_text, selected_position);
			text_changed = true;
		} 

		if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT))) {
			selected_position += 1;
			if (selected_position > strlen(input_text->text)) selected_position = strlen(input_text->text);
			//x_cursor = input_text->rectangle.x + UIGetTextWidthWithLength(input_text, selected_position);
			text_changed = true;
		} 

		if ((IsKeyPressed(KEY_ENTER) || IsKeyPressedRepeat(KEY_ENTER))) {
			input_text->text = string_insert(input_text->text, selected_position, '\n');
			selected_position += 1;
			//x_cursor = input_text->rectangle.x + UIGetTextWidthWithLength(input_text, selected_position);
			text_changed = true;
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			selected_position = SetTextGetCharClosestToPosition(&set_text, (Vector2){.x=GetMouseX(),.y=GetMouseY()});
			//x_cursor = input_text->rectangle.x + UIGetTextWidthWithLength(input_text, selected_position);
			text_changed = true;
			
		}

		if (text_changed) {
			UIConvertTextToSetText(input_text, &set_text);
			cursor = SetTextGetPositionOfChar(&set_text, selected_position);
		}
		DrawLine(cursor.x, cursor.y, cursor.x, cursor.y + input_text->size, WHITE);

		char* input_size;
		asprintf(&input_size, "input size: %d", UIGetTextWidth(input_text));
		DrawText(input_size, 300, 300, 20, WHITE);
		free(input_size);
		char* closest_point;
		size_t closest_char = UIGetCharClosestToPosition(input_text, (Vector2){.x=GetMouseX(),.y=GetMouseY()});
		asprintf(&closest_point, "closest_char: %zu", closest_char);
		DrawText(closest_point, 300, 400, 20, WHITE);
		free(closest_point);

		char* current_char;
		asprintf(&current_char, "current char: %zu", selected_position);
		DrawText(current_char, 300, 500, 20, WHITE);
		free(current_char);

		UIText text;
		bool OnScreen = false;
		if (IsCursorOnScreen()) {
			//OnScreen = true;

			//text = (UIText){.color=WHITE,.fontSize=20,.position={.x=GetMouseX(),.y=GetMouseY()},.text="HI!!!"};
		}


		

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		// draw some text using the default font
		DrawText(!OnScreen ? "Hello Raylib! This is Nemo Dat!" : "ON SCREEN", 200,200,20,WHITE);

		if (OnScreen) {
			//UIDrawText(&text);
		}

		// UIDrawText(input_text);
		// UIDrawBoundedText(input_text,GetFontDefault(),true,input_text->size/10.0);
		
		DrawSetText(&set_text);

		// draw our texture to the screen
		DrawTexture(wabbit, 400, 200, WHITE);
		
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(wabbit);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
