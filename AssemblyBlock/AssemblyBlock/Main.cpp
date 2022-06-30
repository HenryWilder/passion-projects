#include <algorithm>
#include <vector>
#include <queue>
#include <numeric>
#include <raylib.h>
#include <config.h>
#include <raymath.h>
#include <rlgl.h>
#include "external/glad.h"
#include <unordered_map>

#pragma region Shorthands

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

inline void rlVertexV2(Vector2 vert) { rlVertex2f(vert.x, vert.y); }

#pragma endregion

Vector2 Vector2Floor(Vector2 vec)
{
	Vector2 ret = { floorf(vec.x), floorf(vec.y) };
	return ret;
}

void DrawRectangleMinMax(float xMin, float yMin, float xMax, float yMax, Color color)
{
	Vector2 topLeft     = { xMin, yMin };
	Vector2 topRight    = { xMax, yMin };
	Vector2 bottomLeft  = { xMin, yMax };
	Vector2 bottomRight = { xMax, yMax };

#if defined(SUPPORT_QUADS_DRAW_MODE)
	rlCheckRenderBatchLimit(4);

	rlSetTexture(1u);

	rlBegin(RL_QUADS);

	rlNormal3f(0.0f, 0.0f, 1.0f);
	rlColor4ub(color.r, color.g, color.b, color.a);
	rlTexCoord2f(0, 0);
	rlVertexV2(topLeft);
	rlVertexV2(bottomLeft);
	rlVertexV2(bottomRight);
	rlVertexV2(topRight);

	rlEnd();

	rlSetTexture(0);
#else
	rlCheckRenderBatchLimit(6);

	rlBegin(RL_TRIANGLES);

	rlColor4ub(color.r, color.g, color.b, color.a);

	rlVertex2f(topLeft.x, topLeft.y);
	rlVertex2f(bottomLeft.x, bottomLeft.y);
	rlVertex2f(topRight.x, topRight.y);

	rlVertex2f(topRight.x, topRight.y);
	rlVertex2f(bottomLeft.x, bottomLeft.y);
	rlVertex2f(bottomRight.x, bottomRight.y);

	rlEnd();
#endif
}
void DrawRectangleLinesMinMax(float xMin, float yMin, float xMax, float yMax, Color color)
{
	float width = xMax - xMin;
	float height = yMax - yMin;
#if defined(SUPPORT_QUADS_DRAW_MODE)

	DrawRectangleMinMax(xMin, yMin, xMax, 1, color);
	DrawRectangleMinMax(xMin - 1, yMin + 1, 1, yMax - 1, color);
	DrawRectangleMinMax(xMin, yMin + 1, xMax, 1, color);
	DrawRectangleMinMax(xMin, yMin + 1, 1, yMax - 1, color);
#else
	rlBegin(RL_LINES);
	rlColor4ub(color.r, color.g, color.b, color.a);
	rlVertex2i(xMin + 1, yMin + 1);
	rlVertex2i(xMax, yMin + 1);

	rlVertex2i(xMax, yMin + 1);
	rlVertex2i(xMax, yMax);

	rlVertex2i(xMax, yMax);
	rlVertex2i(xMin + 1, yMax);

	rlVertex2i(xMin + 1, yMax);
	rlVertex2i(xMin + 1, yMin + 1);
	rlEnd();
#endif
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
	// Todo
};

std::unordered_map<IVec2, Space*> world;

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

Texture2D texShapes;
Rectangle texShapesRec = { 0,0,1,1 };
void InitShapeTexture()
{
	Image img = GenImageColor(1,1,WHITE);
	texShapes = LoadTextureFromImage(img);
	UnloadImage(img);
}

// A rectangle with extra stuff
struct Rect
{
	float xMin, yMin, xMax, yMax;

	Rect() = default;
	Rect(float sidelength) :
		xMin(0), yMin(0), xMax(sidelength), yMax(sidelength) {}
	Rect(float width, float height) :
		xMin(0), yMin(0), xMax(width), yMax(height) {}
	Rect(float x, float y, float width, float height) :
		xMin(x), yMin(y), xMax(x + width), yMax(y + height) {}

	explicit Rect(const Rectangle& original) :
		xMin(original.x), yMin(original.y), xMax(original.x + original.width), yMax(original.y + original.height) {}

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

	float GetX() const { return xMin; }
	void SetX(float value) { float w = Width; xMin = value; xMax = xMin + w; }
	property_rw(GetX, SetX)	float X;

	float GetY() const { return yMin; }
	void SetY(float value) { float h = Height; yMin = value; yMax = yMin + h; }
	property_rw(GetY, SetY)	float Y;

	float GetWidth() const { return xMax - xMin; }
	void SetWidth (float value) { xMax = value + xMin; }
	property_rw(GetWidth, SetWidth) float Width;

	float GetHeight() const { return yMax - yMin; }
	void SetHeight(float value) { yMax = value + yMin; }
	property_rw(GetHeight, SetHeight) float Height;

	// Top left corner
	Vector2 GetPosition() const { return { xMin, yMin }; }
	// Move rect but keep size
	void SetPosition(Vector2 value)
	{
		Vector2 size = GetSize();
		xMin = value.x;
		yMin = value.y;
		xMax = xMin + size.x;
		yMax = yMin + size.y;
	}
	property_rw(GetPosition, SetPosition) Vector2 Position;

	// Top left corner
	Vector2 GetMinPos() const { return { xMin, yMin }; }
	property_ro(GetMinPos) Vector2 Min;

	// Bottom right corner
	Vector2 GetMaxPos() const { return { xMax, yMax }; }
	property_ro(GetMaxPos) Vector2 Max;

	Vector2 GetCenter()	const { return { (xMin + xMax) * 0.5f, (yMin + yMax) * 0.5f }; }
	// Move rect but keep size
	void SetCenter(Vector2 value) { SetPosition(value - GetExtents()); }
	property_rw(GetCenter, SetCenter) Vector2 Center;

	// Measured from the position
	Vector2 GetSize() const { return { GetWidth(), GetHeight() }; }
	// Measured from the position
	void SetSize(Vector2 value) { xMax = xMin + value.x; yMax = yMin + value.y; }
	property_rw(GetSize, SetSize) Vector2 Size;

	// Half size
	Vector2 GetExtents() const { return GetSize() * 0.5f; }
	property_ro(GetExtents)	Vector2 Extents;

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
	static Vector2 PointToNormalized(Rect rectangle, Vector2 point)
	{
		return (point - rectangle.GetPosition()) / rectangle.GetSize();
	}

	const static Rect Zero;

	inline void Draw(Color color) const
	{
		DrawRectangleMinMax(xMin, yMin, xMax, yMax, color);
	}
	inline void DrawLines(Color color) const
	{
		DrawRectangleLinesMinMax(xMin, yMin, xMax, yMax, color);
	}
};
const Rect Rect::Zero = { 0,0,0,0 };

inline void BeginScissorMode(Rect rect) { BeginScissorMode(rect.X, rect.Y, std::max((int)rect.Width, 0), std::max((int)rect.Height, 0)); }

struct RectOffset
{
	int left, right, top, bottom;

	RectOffset() = default;
	constexpr RectOffset(int left, int right, int top, int bottom) : left(left), right(right), top(top), bottom(bottom) {}

	constexpr int GetVertical() const { return left + right; }
	constexpr int GetHorizontal() const { return top + bottom; }

	property_ro(GetHorizontal) int Horizontal;
	property_ro(GetVertical) int Vertical;

	Rect Add(Rect rect) const
	{
		rect.xMin += left;
		rect.xMax -= right;
		rect.yMin += top;
		rect.yMax -= bottom;
		return rect;
	}
	Rect Remove(Rect rect) const
	{
		rect.xMin -= left;
		rect.xMax += right;
		rect.yMin -= top;
		rect.yMax += bottom;
		return rect;
	}

	static const RectOffset inset;
	static const RectOffset expand;
};
const RectOffset RectOffset::inset = RectOffset(1,1,1,1);
const RectOffset RectOffset::expand = RectOffset(-1,-1,-1,-1);

// Global data for all things contained inside the window
namespace Window
{
	Vector2 mousePos; // Screenspace
	Vector2 windowSize; // Screenspace
	MouseCursor cursor = MOUSE_CURSOR_DEFAULT;

	// This should be called ticking any Panels
	void Update()
	{
		mousePos = GetMousePosition();
		if (IsWindowResized())
			windowSize = { (float)GetRenderWidth(), (float)GetRenderHeight() };
	}
}

class Panel;

// Panels are in order of depth; first will always be focused
std::vector<Panel*> panels;
void SetActivePanel(size_t index)
{
	if (index != 0)
	{
		Panel* panel = panels[index];
		panels.erase(panels.begin() + index);
		panels.insert(panels.begin(), panel);
	}
}
// Prefer MakeActivePanel(size_t) where index is already available.
void SetActivePanel(Panel* panel)
{
	auto it = std::find(panels.begin(), panels.end(), panel);
	_ASSERT(it != panels.end());
	SetActivePanel(it - panels.begin());
}
std::queue<Panel*> panelsToAdd;
std::queue<Panel*> panelsToRemove;
void AddPanelAfterTick(Panel* panel)
{
	panelsToAdd.push(panel);
}
void RemovePanelAfterTick(Panel* panel)
{
	panelsToRemove.push(panel);
}

// Contents that gets put inside a SubWindow
class Frame
{
protected:
	Panel* panel; // The owning panel

public:
	void SetPanel(Panel* panel) { this->panel = panel; }

	virtual void TickPassive() = 0; // Tick without any interaction
	virtual void TickActive() = 0; // Tick with interaction
	virtual void Draw() const = 0;
	virtual const char* GetName() const = 0;
};

// A sub-window framel & panel for a Frame. Can be dragged and resized within the main window and displays contents.
class Panel
{
private:
	Rect rect;
	Rect decorationRect; // Set whenever rect changes
	static constexpr RectOffset border = RectOffset(4,4,4,4); // Offset from rect to the frame
	static constexpr int decorationHeight = 17;
	static constexpr float tabWidth = 100;
	static constexpr Vector2 minSize = {
		tabWidth + border.GetHorizontal(), // Todo: Make min width the size of the x button
		decorationHeight + border.GetVertical()
	};
	size_t tabIndex = 0; // The currently visible tab
	std::vector<Frame*> tabs;
	bool active = false;
	bool beingDragged = false;
	bool draggingTab = false;
	int newTabIndex = -1; // Index of tab that is going to be added - negative for none
	static constexpr int newTabInserterLineWidth = 5;
	Vector2 tabDelta = { 0,0 };
	enum class Axis { negative = -1, null = 0, positive = 1 };
	Axis beingResized_horizontal = Axis::null;
	Axis beingResized_vertical   = Axis::null;

	void UpdateDecorationRect()
	{
		decorationRect = border.Add(rect);
		decorationRect.Height = decorationHeight;
	}

	void MoveViewport(Vector2 delta);

public:
	static Shader ghostShader;
	static Shader gripShader;
	static int gripShaderSizeLoc;

	Rect GetContentRect() const
	{
		Rect contentRect = border.Add(rect);
		contentRect.yMin += decorationHeight + 1;
		return contentRect;
	}
	Frame* GetCurrentTab() { return tabs[tabIndex]; }
	const Frame* GetCurrentTab() const { return tabs[tabIndex]; }

	// Checks if the point is on the panel rect
	bool PanelContains(Vector2 point)
	{
		return rect.Contains(point);
	}
	// Checks if the point is on the content rect
	bool ContentContains(Vector2 point)
	{
		return GetContentRect().Contains(point);
	}
	// Checks if the point is on panel rect but not the content
	bool BorderContains(Vector2 point)
	{
		return PanelContains(point) && !ContentContains(point);
	}

	Panel() = default;
	Panel(Rect rect) : rect(rect) { UpdateDecorationRect(); }
	Panel(Rect rect, Frame* tab) : rect(rect) { UpdateDecorationRect(); AddTab(tab); }

	void AddTab(Frame* tab)
	{
		_ASSERT(tab != nullptr);
		tab->SetPanel(this);
		tabs.push_back(tab);
	}
	void InsertTab(Frame* tab, size_t at)
	{
		_ASSERT(at <= tabs.size());
		_ASSERT(tab != nullptr);
		tab->SetPanel(this);
		tabs.insert(tabs.begin() + at, tab);
	}
	Frame* RemoveTab(size_t at)
	{
		_ASSERT(at < tabs.size());
		Frame* tab = tabs[at];
		tabs.erase(tabs.begin() + at);
		tabIndex = std::min(tabIndex, tabs.size() - 1);
		return tab;
	}

	void UpdateDragAndResize()
	{
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			// Enter a panel
			if (draggingTab)
			{
				Rect tabRect = decorationRect;
				tabRect.Width = tabWidth;
				for (Panel* panel : panels)
				{
					if (panel != this && tabRect.Overlaps(panel->decorationRect))
					{
						Frame* tab = GetCurrentTab();
						panel->AddTab(tab);
						panel->tabIndex = panel->tabs.size() - 1; // Todo: insertion, not just appending
						panel->newTabIndex = -1;
						if (tabs.size() > 1) // I have a family!
						{
							RemoveTab(tabIndex);
							draggingTab = false;
							active = false;
						}
						else // My work here is done.
						{
							RemovePanelAfterTick(this);
						}
						SetActivePanel(panel); // Todo: Uhhh... Could be problematic...
						break;
					}
				}
			}
			beingDragged = false;
			draggingTab = false;
			beingResized_horizontal = Axis::null;
			beingResized_vertical = Axis::null;
			return;
		}

		else if (beingDragged || draggingTab)
		{
			if (beingDragged)
			{
				Vector2 delta = GetMouseDelta();
				rect.Position += delta;
				decorationRect.Position += delta;
				MoveViewport(delta);
			}
			if (draggingTab)
			{
				tabDelta += GetMouseDelta();
				// Want to enter a panel
				Rect tabRect = decorationRect;
				tabRect.Width = tabWidth;
				for (Panel* panel : panels)
				{
					if (panel != this && tabRect.Overlaps(panel->decorationRect))
						panel->newTabIndex = panel->tabs.size();
					else
						panel->newTabIndex = -1;
				}
				// Exit the panel
				if (!beingDragged && !decorationRect.Contains(Window::mousePos))
				{
					Rect newPanelRect = rect;
					newPanelRect.Position += tabDelta;
					newPanelRect.X += tabIndex * tabWidth;
					Frame* tab = GetCurrentTab();
					Panel* newPanel = new Panel(newPanelRect, tab);
					newPanel->beingDragged = true;
					newPanel->draggingTab = true;
					newPanel->active = true;
					AddPanelAfterTick(newPanel);
					RemoveTab(tabIndex);
					draggingTab = false;
					active = false;
				}
			}
		}
		else
		{
			Vector2 actualChange = { 0,0 };
			switch (beingResized_horizontal)
			{
			case Panel::Axis::negative:
				actualChange.x = -rect.xMin;
				rect.xMin = std::min(Window::mousePos.x, rect.xMax - minSize.x);
				actualChange.x += rect.xMin;
				break;

			case Panel::Axis::positive:
				actualChange.x = -rect.xMax;
				rect.xMax = std::max(Window::mousePos.x, rect.xMin + minSize.x);
				actualChange.x += rect.xMax;
				break;

			default: break;
			}

			switch (beingResized_vertical)
			{
			case Panel::Axis::negative:
				actualChange.y = -rect.yMin;
				rect.yMin = std::min(Window::mousePos.y, rect.yMax - minSize.y);
				actualChange.y += rect.yMin;
				break;

			case Panel::Axis::positive:
				actualChange.y = -rect.yMax;
				rect.yMax = std::max(Window::mousePos.y, rect.yMin + minSize.y);
				actualChange.y += rect.yMax;
				break;

			default: break;
			}
			
			Vector2 move = actualChange * Vector2{
				abs((float)beingResized_horizontal),
				abs((float)beingResized_vertical)
			};
			MoveViewport(move * 0.5f);

			UpdateDecorationRect();
		}
	}

	void TickActive()
	{
		active = true;
		UpdateDragAndResize();

		// On content
		if (ContentContains(Window::mousePos))
		{
			GetCurrentTab()->TickActive();
			return;
		}

		// On border/decoration
		if (PanelContains(Window::mousePos))
		{
			Axis canResize_horizontal = Axis::null;
			Axis canResize_vertical = Axis::null;

			// On border
			if (!decorationRect.Contains(Window::mousePos))
			{
				Rect contentRect = border.Add(rect);

				if (Window::mousePos.x < contentRect.xMin)
					canResize_horizontal = Axis::negative;
				else if (Window::mousePos.x > contentRect.xMax)
					canResize_horizontal = Axis::positive;

				if (Window::mousePos.y < contentRect.yMin)
					canResize_vertical = Axis::negative;
				else if (Window::mousePos.y > contentRect.yMax)
					canResize_vertical = Axis::positive;

				if ((canResize_horizontal == Axis::negative && canResize_vertical == Axis::negative) ||
					(canResize_horizontal == Axis::positive && canResize_vertical == Axis::positive))
					Window::cursor = MOUSE_CURSOR_RESIZE_NWSE;
				else if ((canResize_horizontal == Axis::negative && canResize_vertical == Axis::positive) ||
					(canResize_horizontal == Axis::positive && canResize_vertical == Axis::negative))
					Window::cursor = MOUSE_CURSOR_RESIZE_NESW;
				else if (canResize_horizontal != Axis::null)
					Window::cursor = MOUSE_CURSOR_RESIZE_EW;
				else if (canResize_vertical != Axis::null)
					Window::cursor = MOUSE_CURSOR_RESIZE_NS;
			}

			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				// On decoration
				if (decorationRect.Contains(Window::mousePos))
				{
					unsigned clickIndex = (unsigned)((Window::mousePos.x - decorationRect.xMin) / tabWidth);
					if (clickIndex < tabs.size())
					{
						tabIndex = clickIndex;
						if (tabs.size() == 1)
							beingDragged = true;
						draggingTab = true;
						tabDelta = { 0,0 };
					}
					else
						beingDragged = true;
				}
				else if (!decorationRect.Contains(Window::mousePos))
				{
					beingResized_horizontal = canResize_horizontal;
					beingResized_vertical   = canResize_vertical;
				}
			}
		}
		if ((beingResized_horizontal == Axis::negative && beingResized_vertical == Axis::negative) ||
			(beingResized_horizontal == Axis::positive && beingResized_vertical == Axis::positive))
			Window::cursor =MOUSE_CURSOR_RESIZE_NWSE;
		else if ((beingResized_horizontal == Axis::negative && beingResized_vertical == Axis::positive) ||
			(beingResized_horizontal == Axis::positive && beingResized_vertical == Axis::negative))
			Window::cursor = MOUSE_CURSOR_RESIZE_NESW;
		else if (beingResized_horizontal != Axis::null)
			Window::cursor = MOUSE_CURSOR_RESIZE_EW;
		else if (beingResized_vertical != Axis::null)
			Window::cursor = MOUSE_CURSOR_RESIZE_NS;

		GetCurrentTab()->TickPassive();
	}

	void TickPassive()
	{
		active = false;
		UpdateDragAndResize();

		if (PanelContains(Window::mousePos))
		{
			bool mouseIsBlocked = false;
			for (Panel* panel : panels)
			{
				if (panel == this)
					continue;

				if (panel->PanelContains(Window::mousePos))
				{
					mouseIsBlocked = true;
					break;
				}
			}
			// On border
			if (!mouseIsBlocked && BorderContains(Window::mousePos))
			{
				Axis canResize_horizontal = Axis::null;
				Axis canResize_vertical = Axis::null;
				Rect contentRect = border.Add(rect);

				if (Window::mousePos.x < contentRect.xMin)
					canResize_horizontal = Axis::negative;
				else if (Window::mousePos.x > contentRect.xMax)
					canResize_horizontal = Axis::positive;

				if (Window::mousePos.y < contentRect.yMin)
					canResize_vertical = Axis::negative;
				else if (Window::mousePos.y > contentRect.yMax)
					canResize_vertical = Axis::positive;

				if ((canResize_horizontal == Axis::negative && canResize_vertical == Axis::negative) ||
					(canResize_horizontal == Axis::positive && canResize_vertical == Axis::positive))
					Window::cursor = MOUSE_CURSOR_RESIZE_NWSE;
				else if ((canResize_horizontal == Axis::negative && canResize_vertical == Axis::positive) ||
					(canResize_horizontal == Axis::positive && canResize_vertical == Axis::negative))
					Window::cursor = MOUSE_CURSOR_RESIZE_NESW;
				else if (canResize_horizontal != Axis::null)
					Window::cursor = MOUSE_CURSOR_RESIZE_EW;
				else if (canResize_vertical != Axis::null)
					Window::cursor = MOUSE_CURSOR_RESIZE_NS;
			}
		}

		GetCurrentTab()->TickPassive();
	}

	// Draws decoration and content
	void Draw() const
	{
		rect.Draw({ 60, 60, 60, 255 });
		RectOffset::expand.Add(border.Add(rect)).Draw({ 100,100,100,255 });

		// Decoration
		decorationRect.Draw({ 60, 60, 60, 255 });
		Rect tabRect = decorationRect;
		tabRect.Width = tabWidth;
		constexpr int fontSize = 10;
		int textY = rect.yMin + border.top + (border.top + decorationHeight - fontSize) / 2;
		bool breakAfterTab = false;
		for (size_t i = 0; i < tabs.size() && !breakAfterTab; ++i)
		{
			if (tabRect.xMax > decorationRect.xMax)
			{
				tabRect.xMax = decorationRect.xMax;
				breakAfterTab = true;
			}
			if (i == tabIndex) tabRect.Draw(active ? BLUE : Color{ 100,100,100,255 });
			DrawLineV({ tabRect.xMax - 0.5f, tabRect.yMin }, { tabRect.xMax - 0.5f, tabRect.yMax }, { 100,100,100,255 });
			BeginScissorMode(tabRect);
			DrawText(tabs[i]->GetName(), tabRect.xMin + border.left, textY, fontSize, RAYWHITE);
			EndScissorMode();
			tabRect.X += tabWidth;
		}
		if (newTabIndex >= 0)
		{
			Rect inserterRect = {};
			inserterRect.yMin = decorationRect.yMin;
			inserterRect.yMax = decorationRect.yMax;
			inserterRect.xMin = decorationRect.xMin + tabWidth * newTabIndex;
			inserterRect.Width = newTabInserterLineWidth;
			inserterRect.Draw({ 0, 127, 255, 255 });
		}
		Rect gripRect = tabRect;
		gripRect.xMax = decorationRect.xMax;
		gripRect = RectOffset(5,5,5,5).Add(gripRect);
		if (gripRect.Width > 0.0f)
		{
			float size[2] = { gripRect.Width, gripRect.Height };
			SetShaderValue(gripShader, gripShaderSizeLoc, size, SHADER_UNIFORM_VEC2);
			BeginShaderMode(gripShader);
			DrawTexturePro(texShapes, texShapesRec, (Rectangle)gripRect, { 0,0 }, 0.0f, { 100,100,100,255 });
			EndShaderMode();
		}

		// Frame
		Rect contentRect = GetContentRect();
		BeginScissorMode(contentRect);
		ClearBackground({ 40,40,40,255 });
		GetCurrentTab()->Draw();
		EndScissorMode();
	}
};
Shader Panel::ghostShader;
Shader Panel::gripShader;
int Panel::gripShaderSizeLoc;

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
	void MoveViewport(Vector2 delta)
	{
		panelPosition += delta;
	}

	void TickActive() final
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
	void TickPassive() final
	{
		drawHoveredSpace = false;
	}

	void Draw() const final
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

	const char* GetName() const final { return "Viewport"; }
};
void Panel::MoveViewport(Vector2 delta)
{
	for (Frame* frame : tabs)
	{
		if (Viewport* viewport = dynamic_cast<Viewport*>(frame))
			viewport->MoveViewport(delta);
	}
}

// A box for tools and actions
class Toolbox : public Frame
{
public:
	void TickActive() final
	{
		// Todo
	}
	void TickPassive() final
	{
		// Todo
	}

	void Draw() const final
	{
		// Todo
	}

	const char* GetName() const final { return "Toolbox"; }
};

// A framel displaying informations and options for the selection
class Inspector : public Frame
{
public:
	void TickActive() final
	{
		// Todo
	}
	void TickPassive() final
	{
		// Todo
	}

	void Draw() const final
	{
		// Todo
	}

	const char* GetName() const final { return "Inspector"; }
};

// A text output for displaying warnings, errors, assertions, and letting the user know when something needs attention
class Console : public Frame
{
public:
	void TickActive() final
	{
		// Todo
	}
	void TickPassive() final
	{
		// Todo
	}

	void Draw() const final
	{
		// Todo
	}

	const char* GetName() const final { return "Console"; }
};

int main()
{
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
	Vector2 windowSize = { 1280, 720 };
	InitWindow((int)windowSize.x, (int)windowSize.y, "Assembly Block v0.0.1");
	InitShapeTexture();
	Panel::gripShader = LoadShader(0, "grip.frag");
	Panel::gripShaderSizeLoc = GetShaderLocation(Panel::gripShader, "size");
	SetTargetFPS(60);

	// Prep phase
	bool invertScrolling = false;
	UseInvertedScoll(invertScrolling);

	panels.reserve(16); // Max expected before needing to reallocate
	panels.push_back(new Panel(Rect(0,0,1280,80), new Toolbox()));
	panels.push_back(new Panel(Rect(0,80,200,640), new Inspector()));
	panels.push_back(new Panel(Rect(200,80,1080,440), new Viewport()));
	panels.push_back(new Panel(Rect(200,520,1080,200), new Console()));

	while (!WindowShouldClose())
	{
		// Sim phase

		Window::Update();

		// Order
		for (size_t i = 0; i < panels.size(); ++i)
		{
			Panel* panel = panels[i];

			// Focused
			if (panel->PanelContains(Window::mousePos) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				SetActivePanel(i);
				break;
			}
		}

		Window::cursor = MOUSE_CURSOR_DEFAULT;

		panels[0]->TickActive();
		for (size_t i = 1; i < panels.size(); ++i)
		{
			panels[i]->TickPassive();
		}

		// Add and remove panels
		while (!panelsToRemove.empty())
		{
			Panel* panel = panelsToRemove.front();
			auto it = std::find(panels.begin(), panels.end(), panel);
			panels.erase(it);
			delete panel;
			panelsToRemove.pop();
		}
		while (!panelsToAdd.empty())
		{
			panels.insert(panels.begin(), panelsToAdd.front()); // @Speed: this is gonna copy a lot if multiple panels are added in a tick
			panelsToAdd.pop();
		}

		SetMouseCursor(Window::cursor);

		// Draw phase
		BeginDrawing();
		{
			ClearBackground({ 80,80,80,255 });

			for (int i = panels.size() - 1; i >= 0; i--)
			{
				panels[i]->Draw();
			}
		}
		EndDrawing();
	}

	// Cleanup phase

	UnloadShader(Panel::gripShader);
	UnloadTexture(texShapes);

	CloseWindow();

	for (Panel* panel : panels)
	{
		delete panel;
	}

	return 0;
}
