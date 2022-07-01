#include "rayex.h"
#include "Rect.h"
#include "Window.h"
#include "Frame.h"
#include "Panel.h"
#include "Viewport.h"

float positiveScrollIncrement = 2.0f;
float negativeScrollIncrement = 0.5f;
void UseInvertedScoll(bool value)
{
	positiveScrollIncrement = value ? 0.5f : 2.0f;
	negativeScrollIncrement = value ? 2.0f : 0.5f;
}

// An interactive look into the game world
#pragma region Viewport

#pragma region Conversions
Viewport::Screenspace_t Viewport::GetWorldToScreen(Worldspace_t point) const
{
	Screenspace_t pixel = GetWorldToScreen2D(point, viewportCamera);
	return pixel;
}
Viewport::Worldspace_t Viewport::GetScreenToWorld(Screenspace_t pixel) const
{
	Screenspace_t point = GetScreenToWorld2D(pixel, viewportCamera);
	return point;
}

Viewport::GridFract_t Viewport::GetWorldToGridV(Worldspace_t point) const
{
	GridFract_t spacef = point * inverseGridScale; // Don't floor, don't cast
	return spacef;
}
Viewport::Gridspace_t Viewport::GetWorldToGrid(Worldspace_t point) const
{
	GridFract_t spacef = GetWorldToGridV(point);
	Gridspace_t space = Vector2FloorToIVec(spacef); // Floor with cast
	return space;
}
Viewport::Worldspace_t Viewport::GetGridToWorld(Gridspace_t space) const
{
	GridFract_t spacef = (GridFract_t)space;
	Worldspace_t point = spacef * gridWidth;
	return point;
}
Viewport::Worldspace_t Viewport::GetGridToWorldV(GridFract_t spacef) const
{
	Worldspace_t point = spacef * gridWidth;
	return point;
}

Viewport::GridFract_t Viewport::SnapToGrid(GridFract_t spacef) const
{
	GridFract_t floored = Vector2Floor(spacef); // Floor without cast
	return floored;
}

Viewport::Worldspace_t Viewport::SnapWorldToGrid(Worldspace_t point) const
{
	GridFract_t spacef = GetWorldToGridV(point);
	GridFract_t spacef_floored = SnapToGrid(spacef); // Floor without cast
	Worldspace_t point_snapped = GetGridToWorldV(spacef_floored);
	return point_snapped;
}
Viewport::Gridspace_t Viewport::GetScreenToGrid(Screenspace_t pixel) const
{
	Worldspace_t point = GetScreenToWorld(pixel);
	Gridspace_t space = GetWorldToGrid(point);
	return  space;
}
Viewport::GridFract_t Viewport::GetScreenToGridV(Screenspace_t pixel) const
{
	Worldspace_t point = GetScreenToWorld(pixel);
	GridFract_t spacef = GetWorldToGridV(point);
	return  spacef;
}
Viewport::Screenspace_t Viewport::GetGridToScreen(Gridspace_t space) const
{
	Worldspace_t point = GetGridToWorld(space);
	Screenspace_t pixel = GetWorldToScreen(point);
	return  pixel;
}
Viewport::Screenspace_t Viewport::GetGridToScreenV(GridFract_t spacef) const
{
	Worldspace_t point = GetGridToWorldV(spacef);
	Screenspace_t pixel = GetWorldToScreen(point);
	return  pixel;
}
#pragma endregion

void Viewport::MoveViewport(Vector2 delta)
{
	panelPosition += delta;
}

void Viewport::TickActive()
{
	// Update input and resizing things
	{
		if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
		{
			Window::cursor = MOUSE_CURSOR_RESIZE_ALL;
			panelPosition += GetMouseDelta();
		}

		hoveredSpace = GetScreenToGrid(Window::mousePos);
		drawHoveredSpace = true;

		if (float scroll = GetMouseWheelMove(); scroll != 0.0f)
		{
			if (scroll > 0.0f)
				viewportCamera.zoom *= positiveScrollIncrement;
			else if (scroll < 0.0f)
				viewportCamera.zoom *= negativeScrollIncrement;
			viewportCamera.zoom = Clamp(viewportCamera.zoom, 0.0625f, 8.0f);
		}
	}

	{
		// Todo: movement/interaction
	}
}
void Viewport::TickPassive()
{
	drawHoveredSpace = false;
}

void Viewport::Draw() const
{
	ClearBackground({ 40,40,40,255 });

	Rect contentRect = panel->GetContentRect();

	Vector2 gridSpace = { gridWidth,gridWidth };
	Screenspace_t frameMin = contentRect.Min - gridSpace;
	Screenspace_t frameMax = contentRect.Max + gridSpace;
	Worldspace_t renderMin = GetScreenToWorld(frameMin);
	Worldspace_t renderMax = GetScreenToWorld(frameMax);
	renderMin = SnapWorldToGrid(renderMin);
	renderMax = SnapWorldToGrid(renderMax);
	Worldspace_t hoveredSpace_ws = GetGridToWorld(hoveredSpace);

	Color gridpointColor = { 80,80,80,255 };
	BeginMode2D(viewportCamera);
	{
		if (drawHoveredSpace)
			DrawRectangleV(hoveredSpace_ws, { gridWidth, gridWidth }, ColorAlpha(gridpointColor, 0.5f));

		if ((1.0f / viewportCamera.zoom) < (gridWidth / 2))
		{
			Vector2 point = renderMin;
			for (point.y = renderMin.y; point.y <= renderMax.y; point.y += gridWidth)
			{
				for (point.x = renderMin.x; point.x <= renderMax.x; point.x += gridWidth)
				{
					DrawPoint2D(point, 2.0f, gridpointColor, viewportCamera);
				}
			}
			//for (float y = renderMin.y; y <= renderMax.y; y += gridWidth)
			//{
			//	DrawLineV({ renderMin.x, y }, { renderMax.x, y }, gridpointColor);
			//}
			//for (float x = renderMin.x; x <= renderMax.x; x += gridWidth)
			//{
			//	DrawLineV({ x, renderMin.y }, { x, renderMax.y }, gridpointColor);
			//}
		}
		else
		{
			DrawRectangleV(renderMin, renderMax - renderMin, gridpointColor);
		}
	}
	EndMode2D();
}

#pragma endregion
