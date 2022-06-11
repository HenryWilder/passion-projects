#pragma once
#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <set>
#include <vector>
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



class Object;

class ObjectTransform
{
private:
	std::set<ObjectTransform*> children;
	ObjectTransform* parent = nullptr;
	Object* object = nullptr;
	Rectangle bounds = { 0,0,0,0 }; // Local space

public:
	inline auto begin() { return children.begin(); }

	inline auto end() { return children.end(); }
	
private:
	void _RemoveSelfFromParent();
	void _AddSelfToParent();
public:
	void RemoveParent();
	void SetParent(ObjectTransform& newParent);
	void SetParent_KeepWorld(ObjectTransform& newParent);

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

	Vector2 GetLocalPosition(Vector2 anchor = { 0.5f, 0.5f }) const;
	// Recursive
	Vector2 GetWorldPosition(Vector2 anchor = { 0.5f, 0.5f }) const;

	void SetLocalPosition(Vector2 position, Vector2 anchor = { 0.5f, 0.5f });
	// Recursive
	void SetWorldPosition(Vector2 position, Vector2 anchor = { 0.5f, 0.5f });
	// Todo: add function for setting the position relative to the parent, with anchors for both the parent and the child

	// Substantially cheaper than getting the position (world or local)
	// and setting the position to that + your offset.
	// Remember: velocity doesn't care about your position, just your direction & magnitude.
	inline void Offset(Vector2 amount)
	{
		AddRectanglePosition(bounds, amount);
	}
};



class Object
{
public:
	ObjectTransform transform;

	Object() = default;
	Object(ObjectTransform trans) : transform(trans) { transform.SetObject(this); }
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
};

template<class ObjectType>
ObjectTransform& Instantiate(Vector2 position = { 0,0 }, Vector2 anchor = { 0.5f,0.5f })
requires std::is_base_of_v<Object, ObjectType>
{
	ObjectTransform trans;
	trans.SetLocalPosition(position, anchor);
	ObjectType* ret = new ObjectType(trans);
	Data::Persistent::allObjects.push_back(ret);
	return ret->transform;
}
void Destroy(Object* object);



class Hoverable : public Object
{
protected:
	bool hovered;

public:
	Hoverable() = default;
	Hoverable(ObjectTransform trans) : Object(trans), hovered() {}
	~Hoverable() = default;

	virtual void Update() override;
	virtual void Draw() const = 0;
};


// Focuses when clicked inside; loses focus when clicked elsewhere
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
	FocusableBase(ObjectTransform trans) : Hoverable(trans) {}
	~FocusableBase() = default;

	virtual void Update() = 0;
	virtual void Draw() const = 0;

	bool IsFocusable() const;
	void SetFocusable(bool value);
};


// Focuses when clicked inside; loses focus when clicked elsewhere
class Focusable : public FocusableBase
{
public:
	Focusable() = default;
	Focusable(ObjectTransform trans) : FocusableBase(trans) {}
	~Focusable() = default;

	virtual void Update() override;
	virtual void Draw() const = 0;
};


// Focus only lasts as long as the mouse is down
class ADDFocusable : public FocusableBase
{
public:
	ADDFocusable() = default;
	ADDFocusable(ObjectTransform trans) : FocusableBase(trans) {}
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
	Draggable(ObjectTransform trans) : ADDFocusable(trans) {}
	~Draggable() = default;

	virtual void Update() override;
	virtual void Draw() const = 0;

	bool IsDraggable() const;
	void SetDraggable(bool value);
};
