#include <raylib.h>
#include <raymath.h>
#include <unordered_map>

Vector2 operator+(Vector2 a, Vector2 b)		{ return Vector2Add(a, b); }
Vector2 operator-(Vector2 a, Vector2 b)		{ return Vector2Subtract(a, b); }
Vector2 operator*(Vector2 a, Vector2 b)		{ return Vector2Multiply(a, b); }
Vector2 operator/(Vector2 a, Vector2 b)		{ return Vector2Divide(a, b); }
Vector2 operator+(Vector2 a, float b)		{ return Vector2AddValue(a, b); }
Vector2 operator-(Vector2 a, float b)		{ return Vector2SubtractValue(a, b); }
Vector2 operator*(Vector2 a, float b)		{ return Vector2Scale(a, b); }
Vector2 operator/(Vector2 a, float b)		{ return Vector2Scale(a, 1.0f/b); }

Vector2 operator-(Vector2 vec)				{ return Vector2Negate(vec); }
Vector2 operator+(Vector2 vec)				{ return vec; }

Vector2& operator+=(Vector2& a, Vector2 b)	{ return a = Vector2Add(a, b); }
Vector2& operator-=(Vector2& a, Vector2 b)	{ return a = Vector2Subtract(a, b); }
Vector2& operator*=(Vector2& a, Vector2 b)	{ return a = Vector2Multiply(a, b); }
Vector2& operator/=(Vector2& a, Vector2 b)	{ return a = Vector2Divide(a, b); }
Vector2& operator+=(Vector2& a, float b)	{ return a = Vector2AddValue(a, b); }
Vector2& operator-=(Vector2& a, float b)	{ return a = Vector2SubtractValue(a, b); }
Vector2& operator*=(Vector2& a, float b)	{ return a = Vector2Scale(a, b); }
Vector2& operator/=(Vector2& a, float b)	{ return a = Vector2Scale(a, 1.0f/b); }

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
	IVec2(Vector2 vec) : x((HalfLong_t)vec.x), y((HalfLong_t)vec.y) {}
	operator Vector2() const { return { (float)x, (float)y }; }

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

class Space
{

};

std::unordered_map<IVec2, Space*> world;

constexpr float g_gridWidth = 32.0f;
constexpr Vector2 g_gridSpaceSize = { g_gridWidth,g_gridWidth };

constexpr float g_InverseGridScale = 1.0f / g_gridWidth;
Vector2 SnapToGrid(Vector2 point, Camera2D camera)
{
	return Vector2Floor(point * g_InverseGridScale) * g_gridWidth;
}
IVec2 GetWorldToGrid(Vector2 point, Camera2D camera)
{
	return Vector2FloorToIVec(point * g_InverseGridScale);
}
Vector2 GetGridToWorld(IVec2 point, Camera2D camera)
{
	return point * g_gridWidth;
}

int main()
{
	Vector2 windowSize = { 1280, 720 };
	InitWindow((int)windowSize.x, (int)windowSize.y, "Assembly Block v0.0.1");

	SetTargetFPS(60);

	// Prep phase
	Camera2D cam{ { 0,0 }, { 0,0 }, 0.0f, 1.0f };
	auto GetGridSize = [&cam]() { return cam.zoom * g_gridWidth; };
	auto GetFrameSize = [&cam, &windowSize]() { return Vector2Scale(windowSize, cam.zoom); };

	while (!WindowShouldClose())
	{
		// Sim phase

		float gridSize = GetGridSize();
		Vector2 frameSize = GetFrameSize();

		if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
			cam.offset += GetMouseDelta();

		IVec2 mouseGridPos = GetWorldToGrid(GetScreenToWorld2D(GetMousePosition(), cam), cam);

		{
			float scroll = GetMouseWheelMove();
			if (scroll > 0.0f && cam.zoom > FLT_MIN)
				cam.zoom *= 0.5f;
			else if (scroll < 0.0f)
				cam.zoom *= 2.0f;
		}

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			BeginMode2D(cam);
			{
				DrawRectangleV(GetGridToWorld(mouseGridPos, cam), g_gridSpaceSize, ColorAlpha(LIGHTGRAY, 0.5f));

				Vector2 gridpointSize(2.0f, 2.0f);
				Vector2 gridpointHalfSize = gridpointSize * 0.5f;

				Vector2 min = SnapToGrid(-cam.offset, cam) - gridpointHalfSize;
				Vector2 max = SnapToGrid(-cam.offset + frameSize, cam) - gridpointHalfSize;

				Vector2 gp = min;
				for (gp.y = min.y; gp.y <= max.y; gp.y += gridSize)
				{
					for (gp.x = min.x; gp.x <= max.x; gp.x += gridSize)
					{
						DrawRectangleV(gp, gridpointSize, GRAY);
					}
				}
			}
			EndMode2D();
		}
		EndDrawing();
	}

	// Cleanup phase

	CloseWindow();

	return 0;
}
