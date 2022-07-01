#include "Window.h"

namespace Window
{
	Vector2 mousePos; // Screenspace
	Rect windowRect; // Screenspace
	MouseCursor cursor = MOUSE_CURSOR_DEFAULT;

	// This should be called ticking any Panels
	void Update()
	{
		mousePos = GetMousePosition();
		if (IsWindowResized())
			windowRect = { 0, 0, (float)GetRenderWidth(), (float)GetRenderHeight() };
	}
}
