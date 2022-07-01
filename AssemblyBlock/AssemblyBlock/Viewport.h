#pragma once
#include <raylib.h>
#include "Frame.h"
#include "GridWorld.h"

void UseInvertedScoll(bool value);

// An interactive look into the game world
class Viewport : public Frame
{
public:
	// A point in world space
	// Floating point
	using Worldspace_t = Vector2;

	// A point in screen space
	// Floating point
	using Screenspace_t = Vector2;

	// A point in grid space
	// Integer
	using Gridspace_t = IVec2;

	// A point in fractional grid space
	// Floating point
	using GridFract_t = Vector2;

private:
	Gridspace_t hoveredSpace = {}; // Gridspace

	Camera2D viewportCamera{ { 0,0 }, { 0,0 }, 0.0f, 1.0f };
	Worldspace_t& panelPosition = viewportCamera.offset; // Relative to the frame

	Worldspace_t frameSize = { 0,0 };

	bool drawHoveredSpace = false;

public:
	// Width of a gridspace in world units
	static constexpr float gridWidth = 32.0f;
	static constexpr float inverseGridScale = 1.0f / gridWidth;

#pragma region Conversions
	Screenspace_t GetWorldToScreen(Worldspace_t point) const;
	Worldspace_t GetScreenToWorld(Screenspace_t pixel) const;

	GridFract_t GetWorldToGridV(Worldspace_t point) const;
	Gridspace_t GetWorldToGrid(Worldspace_t point) const;
	Worldspace_t GetGridToWorld(Gridspace_t space) const;
	Worldspace_t GetGridToWorldV(GridFract_t spacef) const;

	GridFract_t SnapToGrid(GridFract_t spacef) const;

	Worldspace_t SnapWorldToGrid(Worldspace_t point) const;
	Gridspace_t GetScreenToGrid(Screenspace_t pixel) const;
	GridFract_t GetScreenToGridV(Screenspace_t pixel) const;
	Screenspace_t GetGridToScreen(Gridspace_t space) const;
	Screenspace_t GetGridToScreenV(GridFract_t spacef) const;
#pragma endregion

public:
	void MoveViewport(Vector2 delta);

	void TickActive() final;
	void TickPassive() final;

	void Draw() const final;

	const char* GetName() const final;
};
