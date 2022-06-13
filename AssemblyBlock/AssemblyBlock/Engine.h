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


class ObjectTransform;

// Structure for constructing an ObjectTransform
struct BasicTransform
{
	ObjectTransform* parent = nullptr;
	Vector2 pivot = { .5f, .5f };
	Vector2 position = { 0,0 };
	bool positionIsWorldspace = false;
};

class Object;


class ObjectTransform
{
private:
	ObjectTransform* parent = nullptr;
	Object* object = nullptr;
	Rectangle bounds = { 0,0,0,0 }; // Local space
	Vector2 pivot = { 0,0 };
	std::set<ObjectTransform*> children;

public:
	ObjectTransform() = default;
	ObjectTransform(BasicTransform data, Object* owningObject);
	~ObjectTransform() = default;

public:
	_NODISCARD inline decltype(children)::iterator begin() noexcept { return children.begin(); }
	_NODISCARD inline decltype(children)::const_iterator begin() const noexcept { return children.begin(); }
	_NODISCARD inline decltype(children)::iterator end() noexcept { return children.end(); }
	_NODISCARD inline decltype(children)::const_iterator end() const noexcept { return children.end(); }
	
private:
	void _RemoveSelfFromParent();
	void _AddSelfToParent();
public:
	void RemoveParent();
	void SetParent_KeepLocal(ObjectTransform& newParent);
	void SetParent_KeepWorld(ObjectTransform& newParent);
	void SetParent(ObjectTransform& newParent, bool keepWorld);

	const ObjectTransform* Parent() const;

	void SetObject(Object* object);
	const Object* MyObject() const;

	Vector2 RelativePivotPosition() const;
	Rectangle WorldBounds() const;
	Rectangle LocalBounds() const;
	void SetExtents(Vector2 extents);

	Vector2 Pivot() const;
	void SetPivot(Vector2 pivot);

	Vector2 GetLocalPosition() const;
	// Recursive
	Vector2 GetWorldPosition() const;

	void SetLocalPosition(Vector2 position);
	// Recursive
	void SetWorldPosition(Vector2 position);
	// Todo: add function for setting the position relative to the parent, with anchors for both the parent and the child

	// Substantially cheaper than getting the position (world or local)
	// and setting the position to that + your offset.
	// Remember: velocity doesn't care about your position, just your direction & magnitude.
	inline void Offset(Vector2 amount) { AddRectanglePosition(bounds, amount); }
};

template<class _Eval>
void TraverseFromAncestor(const ObjectTransform* start, _Eval evaluateNode)
requires(std::is_invocable_v<_Eval, const ObjectTransform*>)
{
	if (!start) return;
	std::stack<const ObjectTransform*> route({ start });
	while (auto parent = route.top()->Parent())
	{
		route.push(parent);
	}
	while (!route.empty())
	{
		evaluateNode(route.top());
		route.pop();
	}
}


#pragma region Abstract classes


class Object
{
protected:
	// Whether reverse update should be evaluated before reverse update
	bool _rbf = false;
	friend int main();

	// Update parents before children
	virtual void ForwardUpdate();
	// Update children before parents
	virtual void ReverseUpdate();

public:
	ObjectTransform transform;

	Object() = default;
	Object(BasicTransform trans);
	~Object() = default;

#pragma region Check collision
	// Todo: make a function to get the anchor from a point on the rectangle
	bool CheckPointSimpleCollision(Vector2 point) const;
#if _DEBUG
	// Display that the collision check on this object has been skipped
	void SkipPointSimpleCollision() const;
#endif
	// Make sure to specialize "IsComplexCollisionDifferentFromSimpleCollision"
	// too if you specialize this function, or it will be skipped!!
	virtual bool CheckPointComplexCollision(Vector2 point) const;
	virtual constexpr bool IsComplexCollisionDifferentFromSimpleCollision() const { return false; }
#pragma endregion

	virtual void Draw() const = 0;
	virtual inline const char* GetTypeName() const { return "Base Object"; }
};

template<class ObjectType, typename... Args>
concept ConstructableEngineObject = requires(Args&&... args)
{
	// Must be derivative of the Object base class
	std::is_base_of_v<Object, ObjectType>;
	// Passed arguments must constitute a valid constructor for the derived type
	{ new ObjectType(std::forward<Args>(args)...) } -> std::convertible_to<ObjectType*>;
};

template<class ObjectType, typename... Args>
ObjectType* Instantiate(Args&&... _Val) requires(ConstructableEngineObject<ObjectType, Args...>)
{
	// Reserve place in object list before calling constructor
	// (Constructor might instantiate children, muddling the order)
	ObjectType* ret = new ObjectType(std::forward<Args>(_Val)...);
	Data::Persistent::allObjects.push_back(ret);
	return ret;
}
void Destroy(Object* object);
void SortObjects();



class Hoverable : public Object
{
protected:
	bool hovered = false;
	virtual void OnHover();
	virtual void OnUnhover();
	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	Hoverable() = default;
	Hoverable(BasicTransform trans);
	~Hoverable() = default;
};


class FocusableBase : public Hoverable
{
private:
	bool b_focusable = true;

protected:
	bool focused = false;
	virtual void OnFocus();
	virtual void OnLoseFocus();

public:
	FocusableBase() = default;
	FocusableBase(BasicTransform trans);
	~FocusableBase() = default;

	bool IsFocusable() const;
	void SetFocusable(bool value);
};


// Focuses when clicked inside; loses focus when clicked elsewhere
// Useful for things like dropdowns or windows
class Focusable : public FocusableBase
{
protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	Focusable() = default;
	Focusable(BasicTransform trans);
	~Focusable() = default;
};


// Focus only lasts as long as the mouse is down
// Useful for things like sliders or spinners
class ADDFocusable : public FocusableBase
{
protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	ADDFocusable() = default;
	ADDFocusable(BasicTransform trans);
	~ADDFocusable() = default;
};


class Draggable : public ADDFocusable
{
protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

private:
	bool b_draggable = true;

	void OnFocus() final;
	void OnLoseFocus() final;

protected:
	bool beingDragged = false;
	virtual void OnStartDragging();
	virtual void OnStopDragging();

public:
	Draggable() = default;
	Draggable(BasicTransform trans);
	~Draggable() = default;

	bool IsDraggable() const;
	void SetDraggable(bool value);
};

#pragma endregion
