#include <algorithm>
#include <numeric>
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



#pragma endregion

// Draws a square centered around the point
// Size in screenspace
inline void DrawPoint2D(Vector2 point, float size, Color color, Camera2D camera)
{
	float sizeWS = size / camera.zoom;
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
#define property_ro(GetFunc) __declspec(property(get = GetFunc))
#define property_rw(GetFunc, SetFunc) __declspec(property(get = GetFunc, put = SetFunc))

struct Rect
{
	float xMin, yMin, xMax, yMax;

	Rect() = default;
	Rect(float sidelength) :
		xMin(0), yMin(0), xMax(sidelength), yMax(sidelength) {}
	Rect(float width, float height) :
		xMin(0), yMin(0), xMax(width), yMax(height) {}
	Rect(float x, float y, float width, float height) :
		xMin(xMin), yMin(yMin), xMax(xMax), yMax(yMax) {}

	explicit Rect(const Rectangle& original) :
		xMin(original.x), yMin(original.y), xMax(original.width + original.x), yMax(original.height + original.y) {}

	explicit operator Rectangle() { return { X, Y, Width, Height }; }

	static Rect MinMaxRect(float xmin, float ymin, float xmax, float ymax)
	{
		Rect ret{};
		ret.xMin = xmin;
		ret.xMax = xmax;
		ret.yMin = ymin;
		ret.yMax = ymax;
		return ret;
	}

	float GetX()		const { return xMin; }
	float GetY()		const { return yMin; }
	float GetWidth()	const { return xMax - xMin; }
	float GetHeight()	const { return yMax - yMin; }

	// Top left corner
	Vector2 GetPosition() const { return { xMin, yMin }; }
	Vector2 GetCenter()	const { return { (xMin + xMax) * 0.5f, (yMin + yMax) * 0.5f }; }
	// Measured from the position
	Vector2 GetSize()	const { return { GetWidth(), GetHeight() }; }
	// Half size
	Vector2 GetExtents() const { return GetSize() * 0.5f; }

	void SetX	  (float value) { xMin = value; }
	void SetY	  (float value) { yMin = value; }
	void SetWidth (float value) { xMax = value - xMin; }
	void SetHeight(float value) { yMax = value - yMin; }

	// Move rect but keep size
	void SetPosition(Vector2 value)
	{
		Vector2 size = GetSize();
		xMin = value.x;
		yMin = value.y;
		xMax = xMin + size.x;
		yMax = yMin + size.y;
	}
	// Move rect but keep size
	void SetCenter(Vector2 value) { SetPosition(value - GetExtents()); }
	// Measured from the position
	void SetSize(Vector2 value) { xMax = xMin + value.x; yMax = yMin + value.y; }

	property_rw(GetX, SetX)				float X;
	property_rw(GetY, SetY)				float Y;
	property_rw(GetWidth, SetWidth)		float Width;
	property_rw(GetHeight, SetHeight)	float Height;

	property_rw(GetPosition, SetPosition) Vector2 Position;
	property_rw(GetCenter, SetCenter)	Vector2 Center;

	property_rw(GetSize, SetSize)		Vector2 Size;
	property_ro(GetExtents)				Vector2 Extents;

	bool Contains(Vector2 point) const
	{
		return
			(point.x >= xMin) &&
			(point.x <= xMax) &&
			(point.y >= yMin) &&
			(point.y <= yMax);

	}
	bool Overlaps(Rect other) const
	{
		return
			(xMin < other.xMax && xMax > other.xMin) &&
			(yMin < other.yMax && yMax > other.yMin);
	}

	static Vector2 NormalizedToPoint(Rect rectangle, Vector2 normalizedRectCoordinates)
	{
		return normalizedRectCoordinates * rectangle.GetSize() + rectangle.GetPosition();
	}
	// Clamped
	static Vector2 PointToNormalized(Rect rectangle, Vector2 point)
	{
		return (point - rectangle.GetPosition()) / rectangle.GetSize();
	}

	const static Rect Zero;
};
const Rect Rect::Zero = { 0,0,0,0 };

struct RectOffset
{
	int left, right, top, bottom;

	RectOffset() = default;
	RectOffset(int left, int right, int top, int bottom) : left(left), right(right), top(top), bottom(bottom) {}

	int GetVertical() const { return left + right; }
	int GetHorizontal() const { return top + bottom; }

	property_ro(GetHorizontal) int Horizontal;
	property_ro(GetVertical) int Vertical;

	Rect Add(Rect rect)
	{
		rect.xMin += left;
		rect.xMax -= right;
		rect.yMin += top;
		rect.yMax -= bottom;
		return rect;
	}
	Rect Remove(Rect rect)
	{
		rect.xMin -= left;
		rect.xMax += right;
		rect.yMin -= top;
		rect.yMax += bottom;
		return rect;
	}
};

// A sub-window panel. Can be dragged and resized within the main window
class Pane
{
protected:
	Rect rect;

	static Vector2 mousePos; // Screenspace
	static Vector2 windowSize; // Screenspace

	bool CheckMouseInPane() const { return rect.Contains(mousePos); }

public:
	// This should be called before any ticks
	static void Update()
	{
		if (IsWindowResized())
			windowSize = { (float)GetRenderWidth(), (float)GetRenderHeight() };

		mousePos = GetMousePosition();
	}

	// Base: handles frame/decoration interactions for the pane
	virtual void Tick()
	{

	}
	// Base: draws the frame & decoration of the pane
	virtual void Draw() const
	{

	}
};
Vector2 Pane::mousePos; // Screenspace
Vector2 Pane::windowSize; // Screenspace

// An interactive look into the game world
class Viewport : public Pane
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
	Gridspace_t hoveredSpace; // Gridspace

	Camera2D viewportCamera{ { 0,0 }, { 0,0 }, 0.0f, 1.0f };
	Worldspace_t& framePosition = viewportCamera.offset;
	Worldspace_t frameSize = GetScreenToWorld(windowSize); // Size of the viewport in worldspace

public:
	// Width of a gridspace in world units
	static constexpr float gridWidth = 32.0f;
	static constexpr float inverseGridScale = 1.0f / gridWidth;

#pragma region Conversions
	Screenspace_t GetWorldToScreen(Worldspace_t point) const
	{
		Screenspace_t pixel = GetWorldToScreen2D(point, viewportCamera);
		return pixel;
	}
	Worldspace_t GetScreenToWorld(Screenspace_t pixel) const
	{
		Screenspace_t point = GetScreenToWorld2D(pixel, viewportCamera);
		return point;
	}

	GridFract_t GetWorldToGridV(Worldspace_t point) const
	{
		GridFract_t spacef = point * inverseGridScale; // Don't floor, don't cast
		return spacef;
	}
	Gridspace_t GetWorldToGrid(Worldspace_t point) const
	{
		GridFract_t spacef = GetWorldToGridV(point);
		Gridspace_t space = Vector2FloorToIVec(spacef); // Floor with cast
		return space;
	}
	Worldspace_t GetGridToWorld(Gridspace_t space) const
	{
		GridFract_t spacef = (GridFract_t)space;
		Worldspace_t point = spacef * gridWidth;
		return point;
	}
	Worldspace_t GetGridToWorldV(GridFract_t spacef) const
	{
		Worldspace_t point = spacef * gridWidth;
		return point;
	}

	GridFract_t SnapToGrid(GridFract_t spacef) const
	{
		GridFract_t floored = Vector2Floor(spacef); // Floor without cast
		return floored;
	}

	Worldspace_t SnapWorldToGrid(Worldspace_t point) const
	{
		GridFract_t spacef = GetWorldToGridV(point);
		GridFract_t spacef_floored = SnapToGrid(spacef); // Floor without cast
		Worldspace_t point_snapped = GetGridToWorldV(spacef_floored);
		return point_snapped;
	}
	Gridspace_t GetScreenToGrid(Screenspace_t pixel) const
	{
		Worldspace_t point = GetScreenToWorld(pixel);
		Gridspace_t space = GetWorldToGrid(point);
		return  space;
	}
	GridFract_t GetScreenToGridV(Screenspace_t pixel) const
	{
		Worldspace_t point = GetScreenToWorld(pixel);
		GridFract_t spacef = GetWorldToGridV(point);
		return  spacef;
	}
	Screenspace_t GetGridToScreen(Gridspace_t space) const
	{
		Worldspace_t point = GetGridToWorld(space);
		Screenspace_t pixel = GetWorldToScreen(point);
		return  pixel;
	}
	Screenspace_t GetGridToScreenV(GridFract_t spacef) const
	{
		Worldspace_t point = GetGridToWorldV(spacef);
		Screenspace_t pixel = GetWorldToScreen(point);
		return  pixel;
	}
#pragma endregion

public:
	void Tick() final
	{
		Pane::Tick();

		if (!CheckMouseInPane())
			return;

		// Update input and resizing things
		{
			if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
				framePosition += GetMouseDelta();

			hoveredSpace = GetScreenToGrid(mousePos);

			if (float scroll = GetMouseWheelMove(); scroll != 0.0f)
			{
				float windowMin = std::min(windowSize.x, windowSize.y);
				float zoomMax = windowMin / gridWidth;
				if (scroll > 0.0f && viewportCamera.zoom > FLT_MIN)
					viewportCamera.zoom *= positiveScrollIncrement;
				else if (scroll < 0.0f && viewportCamera.zoom < (gridWidth / 2))
					viewportCamera.zoom *= negativeScrollIncrement;
			}
			frameSize = GetScreenToWorld(rect.Size);
		}

		{
			// Todo: movement/interaction
		}
	}

	void Draw() const final
	{
		Pane::Draw();

		Screenspace_t screenMin = { -gridWidth,-gridWidth };
		Screenspace_t screenMax = windowSize + Vector2{ gridWidth,gridWidth };
		Worldspace_t renderMin = SnapWorldToGrid(GetScreenToWorld(screenMin));
		Worldspace_t renderMax = SnapWorldToGrid(GetScreenToWorld(screenMax));
		Worldspace_t hoveredSpace_ws = GetGridToWorld(hoveredSpace);

		BeginMode2D(viewportCamera);
		{
			DrawRectangleV(hoveredSpace_ws, { gridWidth, gridWidth }, ColorAlpha(LIGHTGRAY, 0.5f));

			if ((1.0f / viewportCamera.zoom) < (gridWidth / 2))
			{
				Vector2 point = renderMin;
				for (point.y = renderMin.y; point.y <= renderMax.y; point.y += gridWidth)
				{
					for (point.x = renderMin.x; point.x <= renderMax.x; point.x += gridWidth)
					{
						DrawPoint2D(point, 2.0f, LIGHTGRAY, viewportCamera);
					}
				}
			}
			else
			{
				DrawRectangleV(renderMin, renderMax - renderMin, LIGHTGRAY);
			}
		}
		EndMode2D();
	}
};

// A bar for tools and actions
class Toolbar : public Pane
{
public:
	void Tick() final
	{
		Pane::Tick();

	}

	void Draw() const final
	{
		Pane::Draw();

	}
};

// A panel displaying informations and options for the selection
class Inspector : public Pane
{
public:
	void Tick() final
	{
		Pane::Tick();

	}

	void Draw() const final
	{
		Pane::Draw();

	}
};

// A text output for displaying warnings, errors, assertions, and letting the user know when something needs attention
class Console : public Pane
{
public:
	void Tick() final
	{
		Pane::Tick();

	}

	void Draw() const final
	{
		Pane::Draw();

	}
};

int main()
{
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
	Vector2 windowSize = { 1280, 720 };
	InitWindow((int)windowSize.x, (int)windowSize.y, "Assembly Block v0.0.1");

	SetTargetFPS(60);

	// Prep phase
	bool invertScrolling = false;
	UseInvertedScoll(invertScrolling);

	std::vector<Viewport> viewports;
	viewports.reserve(4); // Max expected before needing to reallocate
	viewports.push_back({});

	std::vector<Toolbar> toolbars;
	toolbars.reserve(8); // Max expected before needing to reallocate

	std::vector<Inspector> inspectors;
	inspectors.reserve(1); // Max expected before needing to reallocate

	std::vector<Console> consoles;
	consoles.reserve(1); // Max expected before needing to reallocate

	std::vector<const Pane*> drawOrder;

	while (!WindowShouldClose())
	{
		// Sim phase

		Pane::Update();
		for ( Viewport& pane : viewports ) { pane.Tick(); }
		for (  Toolbar& pane : toolbars  ) { pane.Tick(); }
		for (Inspector& pane : inspectors) { pane.Tick(); }
		for (  Console& pane : consoles  ) { pane.Tick(); }

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(DARKGRAY);

			for (const Pane* pane : drawOrder) { pane->Draw(); }

			DrawFPS(0,0);
		}
		EndDrawing();
	}

	// Cleanup phase

	CloseWindow();

	return 0;
}
