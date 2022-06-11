#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <unordered_set>
#include <set>
#include <typeinfo>

// Extend as implemented
inline Vector2 operator-(Vector2 v, float sub) { return Vector2SubtractValue(v, sub); }
inline Vector2 operator*(Vector2 v, float scale) { return Vector2Scale(v, scale); }
inline Vector2 operator+(Vector2 v1, Vector2 v2) { return Vector2Add(v1, v2); }
inline Vector2 operator-(Vector2 v1, Vector2 v2) { return Vector2Subtract(v1, v2); }
inline Vector2 operator*(Vector2 v1, Vector2 v2) { return Vector2Multiply(v1, v2); }
inline Vector2 operator/(Vector2 v1, Vector2 v2) { return Vector2Divide(v1, v2); }

Vector2 LocalFromAnchor(Vector2 extents, Vector2 anchor)
{
	return extents * anchor;
}
Vector2 AnchorFromLocal(Vector2 extents, Vector2 localPos)
{
	Vector2 ret;
	ret.x = ((extents.x == 0) ? (0) : (localPos.x / extents.x));
	ret.y = ((extents.y == 0) ? (0) : (localPos.y / extents.y));
	return ret;
}

Vector2 RectanglePosition(Rectangle rec)
{
	return { rec.x, rec.y };
}
Vector2 RectangleExtents(Rectangle rec)
{
	return { rec.width, rec.height };
}
void SetRectanglePosition(Rectangle& rec, Vector2 pos)
{
	rec.x = pos.x;
	rec.y = pos.y;
}
void AddRectanglePosition(Rectangle& rec, Vector2 offset)
{
	rec.x += offset.x;
	rec.y += offset.y;
}
void SetRectangleExtents(Rectangle& rec, Vector2 ext)
{
	rec.width = ext.x;
	rec.height = ext.y;
}

class Object;

struct ObjectTransform
{
private:
	std::set<ObjectTransform*> children;
	ObjectTransform* parent = nullptr;
	Object* object = nullptr;

public:
	Rectangle bounds = { 0,0,0,0 };

	auto begin()
	{
		return children.begin();
	}
	auto end()
	{
		return children.end();
	}

	void SetParent(ObjectTransform& newParent)
	{
		if (parent)
		{
			auto it = parent->children.find(this);
			_ASSERT_EXPR(it != parent->children.end(), L"Parent must have this as a child");
			parent->children.erase(it);
		}
		parent = &newParent;
		newParent.children.insert(this);
	}
	void SetParent_KeepWorld(ObjectTransform& newParent)
	{
		Vector2 worldPosition = GetWorldPosition();
		SetParent(newParent);
		SetWorldPosition(worldPosition);
	}
	const ObjectTransform* Parent() const
	{
		return parent;
	}

	void SetObject(Object* object)
	{
		this->object = object;
	}
	const Object* Object() const
	{
		return object;
	}

	Vector2 GetLocalPosition(Vector2 anchor = { 0.5f, 0.5f }) const
	{
		return RectanglePosition(bounds) + LocalFromAnchor(RectangleExtents(bounds), anchor);
	}
	// Recursive
	Vector2 GetWorldPosition(Vector2 anchor = { 0.5f, 0.5f }) const
	{
		Vector2 local = GetLocalPosition(anchor);
		if (parent)
			return local + parent->GetWorldPosition();
		else
			return local;
	}

	void SetLocalPosition(Vector2 position, Vector2 anchor = { 0.5f, 0.5f })
	{
		SetRectanglePosition(bounds, position + LocalFromAnchor(RectangleExtents(bounds), anchor));
	}
	// Recursive
	void SetWorldPosition(Vector2 position, Vector2 anchor = { 0.5f, 0.5f })
	{
		Vector2 localPos;
		if (parent)
			localPos = position - parent->GetWorldPosition();
		else
			localPos = position;
		SetLocalPosition(localPos, anchor);
	}

	// Substantially cheaper than getting the position (world or local)
	// and setting the position to that + your offset.
	// Remember: velocity doesn't care about your position, just your direction & magnitude.
	inline void Offset(Vector2 amount)
	{
		AddRectanglePosition(bounds, amount);
	}
};

namespace Data
{
	// Data that gets serialized and reused between program instances
	namespace Serial
	{
		// Todo: put serializable data here

		void Load()
		{
			std::ifstream file("session.ab");
			if (file)
			{
				// Todo
				file.close();
			}
		}
		void Save()
		{

		}
	}

	// Data that lasts across the lifetime of the program
	namespace Persistent
	{
		std::vector<Object*> allObjects;

		void Init()
		{
			// Todo
		}
	};

	// Data that is cleaned and reset at the start of every frame
	namespace Frame
	{
		Vector2 cursor;

		void Init()
		{
			cursor = GetMousePosition();
		}
	};
}

using namespace Data;

class Object
{
public:
	ObjectTransform transform;

	Object() = default;
	Object(ObjectTransform trans) : transform(trans) { transform.SetObject(this); }
	~Object() = default;

#pragma region Check collision
	// Todo: make a function to get the anchor from a point on the rectangle
	bool CheckPointSimpleCollision(Vector2 point) const
	{
		return CheckCollisionPointRec(point, transform.bounds);
	}
	// Make sure to specialize "IsComplexCollisionDifferentFromSimpleCollision"
	// too if you specialize this function, or it will be skipped!!
	virtual bool CheckPointComplexCollision(Vector2 point) const
	{
		return CheckPointSimpleCollision(point);
	}
	virtual constexpr bool IsComplexCollisionDifferentFromSimpleCollision() const { return false; }
	virtual bool CheckPointCollision(Vector2 point) const
	{
		bool colliding = CheckPointSimpleCollision(point);
		if (!colliding) return false;
		if (IsComplexCollisionDifferentFromSimpleCollision())
			colliding &= CheckPointComplexCollision(point);
	}
#pragma endregion

	virtual void Update() = 0;
	virtual void Draw() const = 0;
};

class Hoverable : public Object
{
protected:
	bool hovered;

public:
	Hoverable() = default;
	Hoverable(ObjectTransform trans) : Object(trans), hovered() {}
	~Hoverable() = default;

	virtual void Update() override
	{
		hovered = CheckPointSimpleCollision(Frame::cursor);
		if (!hovered) return;
		if (IsComplexCollisionDifferentFromSimpleCollision())
			hovered &= CheckPointComplexCollision(Frame::cursor);
	}
	virtual void Draw() const = 0;
};

// Focuses when clicked inside; loses focus when clicked elsewhere
class FocusableBase : public Hoverable
{
private:
	bool b_focusable = true;

protected:
	bool focused = false;
	virtual void OnFocus() {}
	virtual void OnLoseFocus() {}

public:
	FocusableBase() = default;
	FocusableBase(ObjectTransform trans) : Hoverable(trans) {}
	~FocusableBase() = default;

	virtual void Update() = 0;
	virtual void Draw() const = 0;

	bool IsFocusable() const
	{
		return b_focusable;
	}
	void SetFocusable(bool value)
	{
		b_focusable = value;
		if (value == false)
			focused = false;
	}
};

// Focuses when clicked inside; loses focus when clicked elsewhere
class Focusable : public FocusableBase
{
public:
	Focusable() = default;
	Focusable(ObjectTransform trans) : FocusableBase(trans) {}
	~Focusable() = default;

	virtual void Update() override
	{
		Hoverable::Update();
		if (!IsFocusable()) return;
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			if (focused = hovered)
				OnFocus();
			else
				OnLoseFocus();
		}
	}
	virtual void Draw() const = 0;
};

// Focus only lasts as long as the mouse is down
class ADDFocusable : public FocusableBase
{
public:
	ADDFocusable() = default;
	ADDFocusable(ObjectTransform trans) : FocusableBase(trans) {}
	~ADDFocusable() = default;

	virtual void Update() override
	{
		Hoverable::Update();
		if (!IsFocusable()) return;
		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
		{
			focused = false;
			OnLoseFocus();
		}
		else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered)
		{
			focused = true;
			OnFocus();
		}
	}
	virtual void Draw() const = 0;
};

class Draggable : public ADDFocusable
{
private:
	bool b_draggable = true;

	void OnFocus() final
	{
		if (!IsDraggable()) return;
		beingDragged = true;
		OnStartDragging();
	}
	void OnLoseFocus() final
	{
		if (!IsDraggable()) return;
		beingDragged = false;
		OnStopDragging();
	}

protected:
	bool beingDragged = false;
	virtual void OnStartDragging() {}
	virtual void OnStopDragging() {}

public:
	Draggable() = default;
	Draggable(ObjectTransform trans) : ADDFocusable(trans) {}
	~Draggable() = default;

	virtual void Update() override
	{
		ADDFocusable::Update();
		beingDragged = focused && b_draggable;
		if (beingDragged)
			transform.Offset(GetMouseDelta());
	}
	virtual void Draw() const = 0;

	bool IsDraggable() const
	{
		return b_draggable;
	}
	void SetDraggable(bool value)
	{
		b_draggable = value;
		if (value == false)
			beingDragged = false;
	}
};

class Pin : public Hoverable
{
public:
	static constexpr Vector2 pinExtents = { 20, 40 };
	static constexpr Color color_basic = GRAY;
	static constexpr Color color_hovered = LIGHTGRAY;

	Pin() = default;
	Pin(ObjectTransform trans) : Hoverable(trans) { SetRectangleExtents(transform.bounds, pinExtents); }
	~Pin() = default;

	void Update() final
	{
		Hoverable::Update();
	}
	void Draw() const final
	{
		DrawRectangleRec(transform.bounds, hovered ? color_hovered : color_basic);
	}
};

class Block : public Draggable
{
public:
	static constexpr Vector2 blockExtents = { 150, 100 };
	static constexpr Color color_basic = GRAY;
	static constexpr Color color_hovered = LIGHTGRAY;
	static constexpr Color color_dragged = LIGHTGRAY;

	Block() = default;
	Block(ObjectTransform trans) : Draggable(trans) { SetRectangleExtents(transform.bounds, blockExtents); }
	~Block() = default;

	void Update() final
	{
		Draggable::Update();
	}
	void Draw() const final
	{
		DrawRectangleRec(transform.bounds, (beingDragged ? color_dragged : (hovered ? color_hovered : color_basic)));
	}
};

template<class T>
concept ObjectDerivative = std::is_base_of_v<Object, T>;

template<ObjectDerivative ObjectType>
ObjectTransform& Instantiate(Vector2 position, Vector2 anchor)
{
	ObjectTransform trans;
	trans.SetLocalPosition(position, anchor);
	ObjectType* ret = new ObjectType(trans);
	Persistent::allObjects.push_back(ret);
	return ret->transform;
}
void Destroy(Object* object)
{
	delete object;
}

int main()
{
	InitWindow(1280, 720, "Assembly Block v0.0.1");

	SetTargetFPS(60);

	// Prep phase

	Serial::Load();
	// Init serial
	{

	}

	Persistent::Init();
	// Init persistent
	{
		decltype(Persistent::allObjects)& objects = Persistent::allObjects;
		auto CreateObject = [](Object* what, Vector2 localPos, Vector2 anchor)
		{
			Persistent::allObjects.push_back(what);
			what->transform.SetLocalPosition(localPos, anchor);
			return &what->transform;
		};
		constexpr float pinWidth = Pin::pinExtents.x;
		constexpr float pinHeight = Pin::pinExtents.y;
		constexpr float blockWidth = Block::blockExtents.x;
		constexpr float blockHeight = Block::blockExtents.y;

		{
			Instantiate<Pin>({ 100, 0 }, { 0, 1 });
			Instantiate<Block>({ 100, 0 }, { 0, 1 });
			Instantiate<Pin>({ 100, 0 }, { 0, 1 });

			CreateObject(new Pin,   { 100, 0 }, { 0, 1 });
			CreateObject(new Block, { 400, 0 }, { 0, 1 });
			CreateObject(new Pin, { 0, blockHeight * 0.5f }, { 0.5f, 0.5f })
				->SetParent(objects[1]->transform);
		}
	}

	while (!WindowShouldClose())
	{
		Data::Frame::Init();

		// Sim phase
		for (Object* obj : Persistent::allObjects)
		{
			obj->Update();
		}

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			for (Object* obj : Persistent::allObjects)
			{
				obj->Draw();
			}
		}
		EndDrawing();
	}

	// Cleanup phase

	CloseWindow();

	return 0;
}
