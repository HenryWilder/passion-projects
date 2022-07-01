#include <raylib.h>
#include "rayex.h"
#include "Rect.h"

#pragma region Rect

Rect::Rect(float sidelength) :
	xMin(0), yMin(0), xMax(sidelength), yMax(sidelength) {}
Rect::Rect(float width, float height) :
	xMin(0), yMin(0), xMax(width), yMax(height) {}
Rect::Rect(float x, float y, float width, float height) :
	xMin(x), yMin(y), xMax(x + width), yMax(y + height) {}

Rect::Rect(const Rectangle& original) :
	xMin(original.x), yMin(original.y), xMax(original.x + original.width), yMax(original.y + original.height) {}

Rect::operator Rectangle() { return { X, Y, Width, Height }; }

Rect Rect::MinMaxRect(float xmin, float ymin, float xmax, float ymax)
{
	Rect ret{};
	ret.xMin = xmin;
	ret.xMax = xmax;
	ret.yMin = ymin;
	ret.yMax = ymax;
	return ret;
}

float Rect::GetX() const { return xMin; }
void Rect::SetX(float value) { float w = Width; xMin = value; xMax = xMin + w; }

float Rect::GetY() const { return yMin; }
void Rect::SetY(float value) { float h = Height; yMin = value; yMax = yMin + h; }

float Rect::GetWidth() const { return xMax - xMin; }
void Rect::SetWidth (float value) { xMax = value + xMin; }

float Rect::GetHeight() const { return yMax - yMin; }
void Rect::SetHeight(float value) { yMax = value + yMin; }

// Top left corner
Vector2 Rect::GetPosition() const { return { xMin, yMin }; }
// Move rect but keep size
void Rect::SetPosition(Vector2 value)
{
	Vector2 size = GetSize();
	xMin = value.x;
	yMin = value.y;
	xMax = xMin + size.x;
	yMax = yMin + size.y;
}

// Top left corner
Vector2 Rect::GetMinPos() const { return { xMin, yMin }; }

// Bottom right corner
Vector2 Rect::GetMaxPos() const { return { xMax, yMax }; }

Vector2 Rect::GetCenter()	const { return { (xMin + xMax) * 0.5f, (yMin + yMax) * 0.5f }; }
// Move rect but keep size
void Rect::SetCenter(Vector2 value) { SetPosition(value - GetExtents()); }

// Measured from the position
Vector2 Rect::GetSize() const { return { GetWidth(), GetHeight() }; }
// Measured from the position
void Rect::SetSize(Vector2 value) { xMax = xMin + value.x; yMax = yMin + value.y; }

// Half size
Vector2 Rect::GetExtents() const { return GetSize() * 0.5f; }

bool Rect::Contains(Vector2 point) const
{
	return
		(point.x >= xMin) &&
		(point.x <= xMax) &&
		(point.y >= yMin) &&
		(point.y <= yMax);

}
bool Rect::Overlaps(Rect other) const
{
	return
		(xMin < other.xMax && xMax > other.xMin) &&
		(yMin < other.yMax && yMax > other.yMin);
}

Vector2 Rect::NormalizedToPoint(Rect rectangle, Vector2 normalizedRectCoordinates)
{
	return normalizedRectCoordinates * rectangle.GetSize() + rectangle.GetPosition();
}
Vector2 Rect::PointToNormalized(Rect rectangle, Vector2 point)
{
	return (point - rectangle.GetPosition()) / rectangle.GetSize();
}

void Rect::Draw(Color color) const
{
	DrawRectangleMinMax(xMin, yMin, xMax, yMax, color);
}
void Rect::DrawLines(Color color) const
{
	DrawRectangleLinesMinMax(xMin, yMin, xMax, yMax, color);
}
const Rect Rect::Zero = { 0,0,0,0 };

void BeginScissorMode(Rect rect)
{
	BeginScissorMode(rect.X, rect.Y, rect.Width, rect.Height);
}

#pragma endregion

#pragma region RectOffset

Rect RectOffset::Add(Rect rect) const
{
	rect.xMin += left;
	rect.xMax -= right;
	rect.yMin += top;
	rect.yMax -= bottom;
	return rect;
}
Rect RectOffset::Remove(Rect rect) const
{
	rect.xMin -= left;
	rect.xMax += right;
	rect.yMin -= top;
	rect.yMax += bottom;
	return rect;
}

const RectOffset RectOffset::inset = RectOffset(1,1,1,1);
const RectOffset RectOffset::expand = RectOffset(-1,-1,-1,-1);

#pragma endregion
