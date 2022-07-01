#pragma once
#include <raylib.h>
#include "Rect.h"

// Global data for all things contained inside the window
namespace Window
{
	extern Vector2 mousePos; // Screenspace
	extern Rect windowRect; // Screenspace
	extern MouseCursor cursor;

	// This should be called ticking any Panels
	void Update();
}
