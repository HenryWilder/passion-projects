#include <raylib.h>
#include <raymath.h>
#include <unordered_map>

#pragma region Vector operators

inline Vector2 operator+(Vector2 a, Vector2 b)		{ return Vector2Add(a, b);				 }
inline Vector2 operator-(Vector2 a, Vector2 b)		{ return Vector2Subtract(a, b);			 }
inline Vector2 operator*(Vector2 a, Vector2 b)		{ return Vector2Multiply(a, b);			 }
inline Vector2 operator/(Vector2 a, Vector2 b)		{ return Vector2Divide(a, b);			 }
inline Vector2 operator+(Vector2 a, float b)		{ return Vector2AddValue(a, b);			 }
inline Vector2 operator-(Vector2 a, float b)		{ return Vector2SubtractValue(a, b);	 }
inline Vector2 operator*(Vector2 a, float b)		{ return Vector2Scale(a, b);			 }
inline Vector2 operator/(Vector2 a, float b)		{ return Vector2Scale(a, 1.0f/b);		 }
inline Vector2 operator+(float a, Vector2 b)		{ return Vector2AddValue(b, a);			 }
inline Vector2 operator-(float a, Vector2 b)		{ return Vector2Subtract({ a,a }, b);	 }
inline Vector2 operator*(float a, Vector2 b)		{ return Vector2Scale(b, a);			 }
inline Vector2 operator/(float a, Vector2 b)		{ return Vector2Divide({ a,a }, b);		 }

inline Vector2 operator-(Vector2 vec)				{ return Vector2Negate(vec);			 }
inline Vector2 operator+(Vector2 vec)				{ return vec;							 }

inline Vector2& operator+=(Vector2& a, Vector2 b)	{ return a = Vector2Add(a, b);			 }
inline Vector2& operator-=(Vector2& a, Vector2 b)	{ return a = Vector2Subtract(a, b);		 }
inline Vector2& operator*=(Vector2& a, Vector2 b)	{ return a = Vector2Multiply(a, b);		 }
inline Vector2& operator/=(Vector2& a, Vector2 b)	{ return a = Vector2Divide(a, b);		 }
inline Vector2& operator+=(Vector2& a, float b)		{ return a = Vector2AddValue(a, b);		 }
inline Vector2& operator-=(Vector2& a, float b)		{ return a = Vector2SubtractValue(a, b); }
inline Vector2& operator*=(Vector2& a, float b)		{ return a = Vector2Scale(a, b);		 }
inline Vector2& operator/=(Vector2& a, float b)		{ return a = Vector2Scale(a, 1.0f/b);	 }

#pragma endregion

Vector2 Vector2Floor(Vector2 vec)
{
	Vector2 ret = { floorf(vec.x), floorf(vec.y) };
	return ret;
}

using HalfLong_t = std::conditional_t<sizeof(size_t) == 8, int, short>;

struct IVec2
{
	IVec2() = default;
	IVec2(int x, int y) : x((HalfLong_t)x), y((HalfLong_t)y) {}
	IVec2(HalfLong_t x, HalfLong_t y) : x(x), y(y) {}
	explicit IVec2(Vector2 vec) : x((HalfLong_t)vec.x), y((HalfLong_t)vec.y) {}
	explicit operator Vector2() const { return { (float)x, (float)y }; }

	HalfLong_t x, y;
};
IVec2 Vector2FloorToIVec(Vector2 vec)
{
	IVec2 ret = { (HalfLong_t)floorf(vec.x), (HalfLong_t)floorf(vec.y) };
	return ret;
}
namespace std
{
	template<>
	struct hash<IVec2>
	{
		static_assert((sizeof(IVec2::x) * 2) == sizeof(size_t));
		_NODISCARD size_t operator()(const IVec2 _Keyval) const noexcept {

			size_t kv =
				static_cast<size_t>(_Keyval.x) << (8 * sizeof(IVec2::x)) |
				static_cast<size_t>(_Keyval.y);
			return _Hash_representation(kv);
		}
	};
}

// Base class for anything that can be on a gridspace
class Space
{

};

Camera2D g_camera{ { 0,0 }, { 0,0 }, 0.0f, 1.0f };
Vector2& g_framePosition = g_camera.offset;
std::unordered_map<IVec2, Space*> world;

#pragma region Space conversions

// Naming convention:
// Gridspace (IVec) = space
// Gridspace (Vec)	= spacef
// Worldspace		= point
// Screenspace		= pixel

// Function naming:
// Get_____ToGrid()  = _____ -> integer grid space
// Get_____ToGridV() = _____ -> fractional grid space

// A point in grid space
// Integer
using Gridspace_t = IVec2;

// A point in fractional grid space
// Floating point
using GridFract_t = Vector2;

// A point in world space
// Floating point
using Worldspace_t = Vector2;

// A point in screen space
// Floating point
using Screenspace_t = Vector2;

// I'm really super counting on you to do your peephole optimization thing, compiler

// Width of a gridspace in world units
constexpr float g_gridWidth = 32.0f;
constexpr float g_InverseGridScale = 1.0f / g_gridWidth;

GridFract_t GetWorldToGridV(Worldspace_t point)
{
	GridFract_t spacef = point * g_InverseGridScale; // Don't floor, don't cast
	return spacef;
}
Gridspace_t GetWorldToGrid(Worldspace_t point)
{
	GridFract_t spacef = GetWorldToGridV(point);
	Gridspace_t space = Vector2FloorToIVec(spacef); // Floor with cast
	return space;
}
Worldspace_t GetGridToWorld(Gridspace_t space)
{
	GridFract_t spacef = (GridFract_t)space;
	Worldspace_t point = spacef * g_gridWidth;
	return point;
}
Worldspace_t GetGridToWorldV(GridFract_t spacef)
{
	Worldspace_t point = spacef * g_gridWidth;
	return point;
}

GridFract_t SnapToGrid(GridFract_t spacef)
{
	GridFract_t floored = Vector2Floor(spacef); // Floor without cast
	return floored;
}

Worldspace_t SnapWorldToGrid(Worldspace_t point)
{
	GridFract_t spacef = GetWorldToGridV(point);
	GridFract_t spacef_floored = SnapToGrid(spacef); // Floor without cast
	Worldspace_t point_snapped = GetGridToWorldV(spacef_floored);
	return point_snapped;
}

Screenspace_t GetWorldToScreen(Worldspace_t point)
{
	Screenspace_t pixel = GetWorldToScreen2D(point, g_camera);
	return pixel;
}
Worldspace_t GetScreenToWorld(Screenspace_t pixel)
{
	Screenspace_t point = GetScreenToWorld2D(pixel, g_camera);
	return point;
}

Gridspace_t GetScreenToGrid(Screenspace_t pixel)
{
	Worldspace_t point = GetScreenToWorld(pixel);
	Gridspace_t space = GetWorldToGrid(point);
	return  space;
}
GridFract_t GetScreenToGridV(Screenspace_t pixel)
{
	Worldspace_t point = GetScreenToWorld(pixel);
	GridFract_t spacef = GetWorldToGridV(point);
	return  spacef;
}
Screenspace_t GetGridToScreen(Gridspace_t space)
{
	Worldspace_t point = GetGridToWorld(space);
	Screenspace_t pixel = GetWorldToScreen(point);
	return  pixel;
}
Screenspace_t GetGridToScreenV(GridFract_t spacef)
{
	Worldspace_t point = GetGridToWorldV(spacef);
	Screenspace_t pixel = GetWorldToScreen(point);
	return  pixel;
}

#pragma endregion

// Draws a square centered around the point
// Size in screenspace
inline void DrawPoint2D(Vector2 point, float size, Color color)
{
	float sizeWS = size / g_camera.zoom;
	float halfSize = sizeWS * 0.5f;
	Vector2 position = point - halfSize;
	DrawRectangleV(position, { sizeWS, sizeWS }, color);
}

float positiveScrollIncrement = 2.0f;
float negativeScrollIncrement = 0.5f;
void UseInvertedScoll(bool value)
{
	positiveScrollIncrement = value ? 0.5f : 2.0f;
	negativeScrollIncrement = value ? 2.0f : 0.5f;
}

int lineIndex = 0;
#define PushDebugText(text, ...) DrawText(TextFormat(text, __VA_ARGS__), 0, 16 * lineIndex++, 8, MAGENTA)

int main()
{
	Vector2 windowSize = { 1280, 720 };
	InitWindow((int)windowSize.x, (int)windowSize.y, "Assembly Block v0.0.1");

	SetTargetFPS(60);

	// Prep phase
	bool invertScrolling = false;
	UseInvertedScoll(invertScrolling);

	while (!WindowShouldClose())
	{
		// Sim phase

		Vector2 mousePos = GetMousePosition();

		if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
			g_framePosition += GetMouseDelta();

		IVec2 hoveredSpace = GetScreenToGrid(mousePos);

		{
			float scroll = GetMouseWheelMove();
			if (scroll > 0.0f && g_camera.zoom > FLT_MIN)
				g_camera.zoom *= positiveScrollIncrement;
			else if (scroll < 0.0f)
				g_camera.zoom *= negativeScrollIncrement;
		}

		Worldspace_t frameSize = GetScreenToWorld(windowSize);

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			size_t gridpointsRendered = 0;

			Screenspace_t screenMin = {128,128};
			Screenspace_t screenMax = windowSize - Vector2{128,128};
			Worldspace_t renderMin = SnapWorldToGrid(GetScreenToWorld(screenMin));
			Worldspace_t renderMax = SnapWorldToGrid(GetScreenToWorld(screenMax));
			Worldspace_t hoveredSpace_ws = GetGridToWorld(hoveredSpace);

			BeginMode2D(g_camera);
			{
				DrawRectangleV(hoveredSpace_ws, { g_gridWidth, g_gridWidth }, ColorAlpha(LIGHTGRAY, 0.5f));

				Vector2 point = renderMin;
				for (point.y = renderMin.y; point.y <= renderMax.y; point.y += g_gridWidth)
				{
					for (point.x = renderMin.x; point.x <= renderMax.x; point.x += g_gridWidth)
					{
						DrawPoint2D(point, 2.0f, GRAY);
						++gridpointsRendered;
					}
				}
			}
			EndMode2D();

			lineIndex = 0;

			PushDebugText("Gridpoints being rendered: %i", gridpointsRendered);
			PushDebugText("Min and max screen coords: (%f, %f)-(%f, %f)", screenMin.x, screenMin.y, screenMax.x, screenMax.y);
			PushDebugText("Hovered coords: (%i, %i)", hoveredSpace.x, hoveredSpace.y);
		}
		EndDrawing();
	}

	// Cleanup phase

	CloseWindow();

	return 0;
}
