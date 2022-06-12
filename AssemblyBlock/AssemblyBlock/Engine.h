#pragma once
#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <set>
#include <vector>
#include <functional>
#include "Data.h"


// Extend as implemented
inline Vector2 operator-(Vector2 v, float sub) { return Vector2SubtractValue(v, sub); }
inline Vector2 operator*(Vector2 v, float scale) { return Vector2Scale(v, scale); }
inline Vector2 operator+(Vector2 v1, Vector2 v2) { return Vector2Add(v1, v2); }
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
	ObjectTransform(BasicTransform data);
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

private:
	// Recursive
	Vector2 _GetAnchorlessWorldPosition() const;
public:
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


#pragma region Abstract classes


class Object
{
public:
	ObjectTransform transform;

	Object() = default;
	Object(BasicTransform trans);
	~Object() = default;

#pragma region Check collision
	// Todo: make a function to get the anchor from a point on the rectangle
	bool CheckPointSimpleCollision(Vector2 point) const;
	// Make sure to specialize "IsComplexCollisionDifferentFromSimpleCollision"
	// too if you specialize this function, or it will be skipped!!
	virtual bool CheckPointComplexCollision(Vector2 point) const;
	virtual constexpr bool IsComplexCollisionDifferentFromSimpleCollision() const { return false; }
#pragma endregion

	virtual void Update() = 0;
	virtual void Draw() const = 0;
	virtual const char* GetTypeName() const = 0;
};

template<class ObjectType, typename... Args>
ObjectTransform& Instantiate(Args&&... _Val)
	requires std::is_base_of_v<Object, ObjectType>
{
	ObjectType* ret = new ObjectType(std::forward<Args>(_Val)...);
	Data::Persistent::allObjects.push_back(ret);
	return ret->transform;
}
void Destroy(Object* object);
void SortObjects();



class Hoverable : public Object
{
protected:
	bool hovered;
	virtual void OnHover();
	virtual void OnUnhover();

public:
	Hoverable() = default;
	Hoverable(BasicTransform trans);
	~Hoverable() = default;

	virtual void Update() override;
	virtual void Draw() const = 0;
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

	virtual void Update() = 0;
	virtual void Draw() const = 0;

	bool IsFocusable() const;
	void SetFocusable(bool value);
};


// Focuses when clicked inside; loses focus when clicked elsewhere
// Useful for things like dropdowns or windows
class Focusable : public FocusableBase
{
public:
	Focusable() = default;
	Focusable(BasicTransform trans);
	~Focusable() = default;

	virtual void Update() override;
	virtual void Draw() const = 0;
};


// Focus only lasts as long as the mouse is down
// Useful for things like sliders or spinners
class ADDFocusable : public FocusableBase
{
public:
	ADDFocusable() = default;
	ADDFocusable(BasicTransform trans);
	~ADDFocusable() = default;

	virtual void Update() override;
	virtual void Draw() const = 0;
};


class Draggable : public ADDFocusable
{
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

	virtual void Update() override;
	virtual void Draw() const = 0;

	bool IsDraggable() const;
	void SetDraggable(bool value);
};

#pragma endregion
