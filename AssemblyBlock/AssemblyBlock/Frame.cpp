#include "rayex.h"
#include "Frame.h"
#include "Panel.h"

void Frame::DrawText(const char* text, int x, int y, int size, Color color)
{
	Rect rect = panel->GetContentRect(); // @Speed: This could get cached

	DrawText(text, rect.xMin + x, rect.yMin + y, size, color);
}

void Frame::DrawRect(float xmin, float ymin, float xmax, float ymax, Color color)
{
	Rect rect = panel->GetContentRect(); // @Speed: This could get cached

	DrawRectangleMinMax(rect.xMin + xmin, rect.yMin + ymin, rect.xMax + xmax, rect.yMax + ymax, color);
}

void Frame::SetPanel(Panel* panel) { this->panel = panel; }
