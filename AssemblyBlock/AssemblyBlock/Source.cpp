#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <unordered_set>
#include <typeinfo>

// Extend as implemented
inline Vector2 operator-(Vector2 v, float sub) { return Vector2SubtractValue(v, sub); }
inline Vector2 operator*(Vector2 v, float scale) { return Vector2Scale(v, scale); }
inline Vector2 operator+(Vector2 v1, Vector2 v2) { return Vector2Add(v1, v2); }
inline Vector2 operator-(Vector2 v1, Vector2 v2) { return Vector2Subtract(v1, v2); }
inline Vector2 operator*(Vector2 v1, Vector2 v2) { return Vector2Multiply(v1, v2); }

Vector2 DeltaFromAnchor(Vector2 extents, Vector2 anchor)
{
	return extents * (anchor - 0.5f);
}

Rectangle RectangleFromTopLeftAndExtents(Vector2 pt, Vector2 ext)
{
	return { pt.x, pt.y, ext.x, ext.y };
}
Rectangle RectangleFromCenterAndExtents(Vector2 pt, Vector2 ext)
{
	return RectangleFromTopLeftAndExtents(pt - ext * 0.5f, ext);
}
template<float width, float height>
Rectangle RectangleFromCenterAndExtents(Vector2 pt)
{
	constexpr Vector2 ext = Vector2{ width, height };
	constexpr Vector2 halfExt = Vector2{ ext.x * 0.5f, ext.y * 0.5f };
	return RectangleFromTopLeftAndExtents(pt - halfExt, ext);
}

class Object;
class Pin;
class Block;

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
protected:
	Vector2 position;
	Vector2 extents;

public:
	Object() = default;
	~Object() = default;
	Object(Vector2 position, Vector2 extents) :
		position(position), extents(extents) {}
	Object(Vector2 anchor, Vector2 position, Vector2 extents) :
		position(), extents(extents)
	{
		SetPositionWithAnchor(anchor, position);
	}

	Vector2 GetPosition() const { return position; }
	// Uses center
	void SetPosition(Vector2 position) { this->position = position; }
	virtual Rectangle GetBoundingBox() const
	{
		return RectangleFromCenterAndExtents(position, extents);
	}
	// Anchor is 0..1
	virtual void SetPositionWithAnchor(Vector2 anchor, Vector2 position)
	{
		this->position = position + DeltaFromAnchor(extents, anchor);
	}
	// Todo: make a function to get the anchor from a point on the rectangle
	void Move(Vector2 delta) { position = position + delta; }
	bool CheckPointSimpleCollision(Vector2 point) const
	{
		return CheckCollisionPointRec(point, GetBoundingBox());
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
		if (!colliding) return;
		if (IsComplexCollisionDifferentFromSimpleCollision())
			colliding &= CheckPointComplexCollision(point);
	}
	virtual void Update() = 0;
	virtual void Draw() const = 0;
};

class Hoverable : public Object
{
protected:
	bool hovered;

public:
	Hoverable() = default;
	~Hoverable() = default;
	Hoverable(Vector2 position, Vector2 extents) :
		Object(position, extents),
		hovered() {}
	Hoverable(Vector2 anchor, Vector2 position, Vector2 extents) :
		Object(anchor, position, extents),
		hovered() {}

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
	~FocusableBase() = default;
	FocusableBase(Vector2 position, Vector2 extents) :
		Hoverable(position, extents) {}
	FocusableBase(Vector2 anchor, Vector2 position, Vector2 extents) :
		Hoverable(anchor, position, extents) {}

	virtual void Update() = 0;
	virtual void Draw() const = 0;

	bool IsFocusable() const
	{
		return b_focusable;
	}
	void SetFocusable(bool value)
	{
		b_focusable = value;
		focused = false;
	}
};

// Focuses when clicked inside; loses focus when clicked elsewhere
class Focusable : public FocusableBase
{
public:
	Focusable() = default;
	~Focusable() = default;
	Focusable(Vector2 position, Vector2 extents) :
		FocusableBase(position, extents)  {}
	Focusable(Vector2 anchor, Vector2 position, Vector2 extents) :
		FocusableBase(anchor, position, extents)  {}

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
	~ADDFocusable() = default;
	ADDFocusable(Vector2 position, Vector2 extents) :
		FocusableBase(position, extents)  {}
	ADDFocusable(Vector2 anchor, Vector2 position, Vector2 extents) :
		FocusableBase(anchor, position, extents)  {}

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
	Vector2 dragOffset;

protected:
	bool beingDragged = false;

public:
	bool b_draggable = true;

	Draggable() = default;
	~Draggable() = default;
	Draggable(Vector2 position, Vector2 extents) :
		ADDFocusable(position, extents)  {}
	Draggable(Vector2 anchor, Vector2 position, Vector2 extents) :
		ADDFocusable(anchor, position, extents)  {}

	virtual void Update() override
	{
		ADDFocusable::Update();
		beingDragged = focused && b_draggable;
		if (beingDragged)
			SetPosition(Frame::cursor);
	}
	virtual void Draw() const = 0;
};

class Pin : public Hoverable
{
	constexpr static Vector2 pinExtents = { 20, 40 };
	constexpr static Color color = GRAY;

public:
	Pin() = default;
	~Pin() = default;
	Pin(Vector2 position) :
		Hoverable(position, pinExtents) {}
	Pin(Vector2 anchor, Vector2 position) :
		Hoverable(anchor, position, pinExtents) {}

	Rectangle GetBoundingBox() const final
	{
		return RectangleFromCenterAndExtents<pinExtents.x, pinExtents.y>(position);
	}
	void Update() final
	{
		Hoverable::Update();
	}
	void Draw() const final
	{
		DrawRectangleRec(GetBoundingBox(), color);
	}
};

class Block : public Draggable
{
	constexpr static Vector2 blockExtents = { 150, 100 };
	constexpr static Color color = GRAY;

public:
	Block() = default;
	~Block() = default;
	Block(Vector2 position) :
		Draggable(position, blockExtents) {}
	Block(Vector2 anchor, Vector2 position) :
		Draggable(anchor, position, blockExtents) {}

	Rectangle GetBoundingBox() const final
	{
		return RectangleFromCenterAndExtents<blockExtents.x, blockExtents.y>(position);
	}
	void Update() final
	{
		Draggable::Update();
	}
	void Draw() const final
	{
		DrawRectangleRec(GetBoundingBox(), color);
	}
};

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
		Persistent::allObjects.push_back(new Pin(Vector2{ 0,1 }, Vector2{ 100, 0 }));
	}

	while (!WindowShouldClose())
	{
		Data::Frame::Init();

		// Sim phase
		{
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				for (Object* obj : Persistent::allObjects)
				{
					obj->SetPosition(Frame::cursor);
				}
			}
			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
			{

			}
		}

		for (Object* obj : Persistent::allObjects)
		{
			obj->Update();
		}

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(BLACK);

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
