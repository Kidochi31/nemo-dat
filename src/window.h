
#pragma once
#include "raylib.h"

void WindowSetup(const char *title,  const char *image) {
    // Create the window and OpenGL context
	InitWindow(800, 600, title);

    Image icon = LoadImage(image);
	ImageFormat(&icon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	SetWindowIcon(icon);
	UnloadImage(icon);
}