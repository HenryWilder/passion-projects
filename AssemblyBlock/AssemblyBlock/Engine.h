#pragma once
#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <set>
#include <vector>
#include <functional>
#include <stack>
#include <queue>
#include "Data.h"


#if _DEBUG
struct DebugPoint { Vector2 point; Color color = MAGENTA; };
struct DebugCircle { Vector2 point; float radius; int segments = 12;  float thickness = 1; Color color = MAGENTA; };
struct DebugLine { Vector2 start; Vector2 end; float thickness = 1; Color color = MAGENTA; };
struct DebugRectangle { Rectangle rec; float thickness = 1; Color color = MAGENTA; };
namespace Debug
{
	struct DebugShape
	{
		Color color;
		DebugShape(Color color) :
			color(color) {}
		virtual void Draw() const = 0;
	};
	struct Point : public DebugShape
	{
		Vector2 center;
		Point(DebugPoint data) :
			DebugShape(data.color),
			center(data.point) {}
		inline void Draw() const override
		{
			constexpr int crosshairRadius = 8;
			constexpr float circleRadius = 4.0f;
			int x = (int)roundf(center.x);
			int y = (int)roundf(center.y);
			DrawLine(x - crosshairRadius, y, x + crosshairRadius, y, color);
			DrawLine(x, y - crosshairRadius, x, y + crosshairRadius, color);
			DrawRing({ (float)x, (float)y }, circleRadius - 0.5f, circleRadius + 0.5f, 0.0f, 360.0f, 36, color);
		}
	};
	struct Circle : public Point
	{
		float radius;
		float thickness;
		int segments;
		Circle(DebugCircle data) :
			Point({ data.point, data.color }),
			radius(data.radius), thickness(data.thickness), segments(data.segments) {}
		inline void Draw() const final
		{
			float halfThick = thickness * 0.5f;
			DrawRing(center, radius - halfThick, radius + halfThick, 0.0f, 360.0f, segments, color);
		}
	};
	struct Line : public DebugShape
	{
		Vector2 start;
		Vector2 end;
		float thickness;
		Line(DebugLine data) :
			DebugShape(data.color),
			start(data.start), end(data.end), thickness(data.thickness) {}
		inline void Draw() const final
		{
			DrawLineEx(start, end, thickness, color);
		}
	};
	struct Rect : public DebugShape
	{
		Rectangle rec;
		float thickness;
		Rect(DebugRectangle data) :
			DebugShape(data.color),
			rec(data.rec), thickness(data.thickness) {}
		inline void Draw() const final
		{
			DrawRectangleLinesEx(rec, thickness, color);
		}
	};

	extern std::queue<DebugShape*> _debugDrawQueue;
}
inline void DrawDebugPoint(DebugPoint data)
{
	Debug::_debugDrawQueue.push(new Debug::Point(data));
}
inline void DrawDebugCircle(DebugCircle data)
{
	Debug::_debugDrawQueue.push(new Debug::Circle(data));
}
inline void DrawDebugLine(DebugLine data)
{
	Debug::_debugDrawQueue.push(new Debug::Line(data));
}
inline void DrawDebugRect(DebugRectangle data)
{
	Debug::_debugDrawQueue.push(new Debug::Rect(data));
}

void _DrawDebug();
#else
#define DrawDebugPoint(data)
#define DrawDebugCircle(data)
#define DrawDebugCircle(data)
#define DrawDebugLine(data)
#define DrawDebugRect(data)

#define _DrawDebug()
#endif


// Extend as implemented
inline Vector2 operator-(Vector2 v, float sub) { return Vector2SubtractValue(v, sub); }
inline Vector2 operator*(Vector2 v, float scale) { return Vector2Scale(v, scale); }
inline Vector2 operator+(Vector2 v1, Vector2 v2) { return Vector2Add(v1, v2); }
inline Vector2& operator+=(Vector2& v1, Vector2 v2) { return v1 = Vector2Add(v1, v2); }
inline Vector2 operator-(Vector2 v1, Vector2 v2) { return Vector2Subtract(v1, v2); }
inline Vector2 operator*(Vector2 v1, Vector2 v2) { return Vector2Multiply(v1, v2); }
inline Vector2 operator/(Vector2 v1, Vector2 v2) { return Vector2Divide(v1, v2); }

Vector2 LocalFromAnchor(Vector2 extents, Vector2 anchor);
Vector2 AnchorFromLocal(Vector2 extents, Vector2 localPos);

Vector2 RectanglePosition(Rectangle rec);
Vector2 RectangleExtents(Rectangle rec);
void SetRectanglePosition(Rectangle& rec, Vector2 pos);
void AddRectanglePosition(Rectangle& rec, Vector2 offset);
void SetRectangleExtents(Rectangle& rec, Vector2 ext);

class DelegateBase
{
public:
	virtual void Invoke() = 0;
};

template<class _Callable, typename... TEventArgs>
requires std::is_invocable_v<_Callable, TEventArgs...>
class Delegate : public DelegateBase
{
private:
	_Callable func;
public:
	Delegate(_Callable func) : func(func) {}
	void Invoke(TEventArgs... _Val) final { func(_Val...); }
};

template<class _Callable>
requires std::is_invocable_v<_Callable>
class Delegate<_Callable> : public DelegateBase
{
private:
	_Callable func;
public:
	Delegate(_Callable func) : func(func) {}
	void Invoke() final { func(); }
};

template<class T, class _Callable, typename... TEventArgs>
requires std::is_class_v<T> && std::is_member_pointer_v<_Callable> && std::is_invocable_v<_Callable, TEventArgs...>
class MemberDelegate : public DelegateBase
{
private:
	T caller;
	_Callable func;
public:
	MemberDelegate(T caller, _Callable func) : caller(caller), func(func) {}
	void Invoke(TEventArgs... _Val) final { caller->func(_Val...); }
};

template<class T, class _Callable>
requires std::is_class_v<T> && std::is_member_pointer_v<_Callable> && std::is_invocable_v<_Callable>
class MemberDelegate : public DelegateBase
{
private:
	T caller;
	_Callable func;
public:
	MemberDelegate(T caller, _Callable func) : caller(caller), func(func) {}
	void Invoke() final { caller->func(); }
};

template<typename... TEventArgs>
class Event
{
private:
	std::vector<DelegateBase*> delegates;

public:
	~Event() { DisconnectAll(); }

	// Returns the index to use for disconnecting
	template<class _Callable>
	requires std::is_invocable_v<_Callable, TEventArgs...>
	size_t Connect(_Callable func)
	{
		delegates.push_back(new Delegate<_Callable, TEventArgs>(func));
		return delegates.size() - 1;
	}
	template<class T, class _Callable>
	requires std::is_invocable_v<_Callable, TEventArgs...>
	size_t Connect(T caller, _Callable func)
	{
		delegates.push_back(new MemberDelegate<T, _Callable, TEventArgs>(func));
		return delegates.size() - 1;
	}
	inline void Disconnect(size_t index)
	{
		delete delegates[index];
		delegates.erase(delegates.begin() + index);
	}
	inline void DisconnectAll()
	{
		for (DelegateBase* d : delegates)
		{
			delete d;
		}
		delegates.clear();
	}
	inline void Invoke(TEventArgs... args)
	{
		for (DelegateBase* d : delegates)
		{
			d->Invoke(args...);
		}
	}
};

template<>
class Event<>
{
private:
	std::vector<DelegateBase*> delegates;

public:
	~Event() { DisconnectAll(); }

	// Returns the index to use for disconnecting
	template<class _Callable>
	requires std::is_invocable_v<_Callable>
	size_t Connect(_Callable func)
	{
		delegates.push_back(new Delegate<_Callable>(func));
		return delegates.size() - 1;
	}
	inline void Disconnect(size_t index)
	{
		delete delegates[index];
		delegates.erase(delegates.begin() + index);
	}
	inline void DisconnectAll()
	{
		for (DelegateBase* d : delegates)
		{
			delete d;
		}
		delegates.clear();
	}
	inline void Invoke()
	{
		for (DelegateBase* d : delegates)
		{
			d->Invoke();
		}
	}
};

class UIRectangle
{
private:
	Rectangle rect;
	bool enabled = false;
	bool mouseInside = false;
	bool mousePressedMeButHasntReleased = false;

public:
	Event<> MouseEnterEvent;
	Event<> MouseLeaveEvent;
	Event<Vector2> MousePressEvent;
	Event<Vector2> MouseReleaseEvent;

	void PollMouseHover(Vector2 position)
	{

	}

	void SetEnabled(bool enabled)
	{
		if (this->enabled == enabled) return;
		this->enabled = enabled;
		if (this->enabled)
			MouseMovedEvent.Connect(this, PollMouseHover);
		else
			MouseMovedEvent.Disconnect(PollMouseHover);
	}

	void PollEvents()
	{
		// Mouse enter/leave
		if (Data::Frame::cursorMoved)
		{
			bool mouseCollision = CheckCollisionPointRec(Data::Frame::cursor, rect);
			if (mouseInside != mouseCollision)
			{
				if (mouseInside = mouseCollision) MouseEnterEvent.Invoke();
				else                              MouseLeaveEvent.Invoke();
			}
		}
		// Mouse press (only when already inside)
		if (mouseInside && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			mousePressedMeButHasntReleased = true;
			MousePressEvent.Invoke(Data::Frame::cursor);
		}
		// Mouse release (only when previously pressed inside)
		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && mousePressedMeButHasntReleased)
		{
			mousePressedMeButHasntReleased = false;
			MouseReleaseEvent.Invoke(Data::Frame::cursor);
		}
	}
};
