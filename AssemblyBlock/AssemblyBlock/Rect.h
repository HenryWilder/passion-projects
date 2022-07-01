#pragma once
#include <raylib.h>
#include "Properties.h"

// A rectangle with extra stuff
struct Rect
{
	float xMin, yMin, xMax, yMax;

	Rect() = default;
	Rect(float sidelength);
	Rect(float width, float height);
	Rect(float x, float y, float width, float height);

	explicit Rect(const Rectangle& original);

	explicit operator Rectangle();

	static Rect MinMaxRect(float xmin, float ymin, float xmax, float ymax);

	float GetX() const;
	void SetX(float value);
	property_rw(GetX, SetX)	float X;

	float GetY() const;
	void SetY(float value);
	property_rw(GetY, SetY)	float Y;

	float GetWidth() const;
	void SetWidth(float value);
	property_rw(GetWidth, SetWidth) float Width;

	float GetHeight() const;
	void SetHeight(float value);
	property_rw(GetHeight, SetHeight) float Height;

	// Top left corner
	Vector2 GetPosition() const;
	// Move rect but keep size
	void SetPosition(Vector2 value);
	property_rw(GetPosition, SetPosition) Vector2 Position;

	// Top left corner
	Vector2 GetMinPos() const;
	property_ro(GetMinPos) Vector2 Min;

	// Bottom right corner
	Vector2 GetMaxPos() const;
	property_ro(GetMaxPos) Vector2 Max;

	Vector2 GetCenter()	const;
	// Move rect but keep size
	void SetCenter(Vector2 value);
	property_rw(GetCenter, SetCenter) Vector2 Center;

	// Measured from the position
	Vector2 GetSize() const;
	// Measured from the position
	void SetSize(Vector2 value);
	property_rw(GetSize, SetSize) Vector2 Size;

	// Half size
	Vector2 GetExtents() const;
	property_ro(GetExtents)	Vector2 Extents;

	bool Contains(Vector2 point) const;
	bool Overlaps(Rect other) const;

	static Vector2 NormalizedToPoint(Rect rectangle, Vector2 normalizedRectCoordinates);
	static Vector2 PointToNormalized(Rect rectangle, Vector2 point);

	const static Rect Zero;

	void Draw(Color color) const;
	void DrawLines(Color color) const;
};

void BeginScissorMode(Rect rect);

struct RectOffset
{
	int left, right, top, bottom;

	RectOffset() = default;
	constexpr RectOffset(int left, int right, int top, int bottom) : left(left), right(right), top(top), bottom(bottom) {}

	constexpr int GetVertical() const { return left + right; }
	constexpr int GetHorizontal() const { return top + bottom; }

	property_ro(GetHorizontal) int Horizontal;
	property_ro(GetVertical) int Vertical;

	Rect Add(Rect rect) const;
	Rect Remove(Rect rect) const;

	static const RectOffset inset;
	static const RectOffset expand;
};
