#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <unordered_set>

Vector2 DeltaFromAnchor(Rectangle bounds, Vector2 anchor)
{
	return {
		bounds.width * (anchor.x - 0.5f),
		bounds.height * (anchor.y - 0.5f)
	};
}

class Object
{
protected:
	Vector2 position;

public:
	Object() = default;
	Object(Vector2 position) : position(position) {}
	Object(Vector2 anchor, Vector2 position) { SetPositionWithAnchor(anchor, position); }
	~Object() = default;
	Vector2 GetPosition() const { return position; }
	// Uses center
	void SetPosition(Vector2 position) { position = position; }
	// Anchor is 0..1
	void SetPositionWithAnchor(Vector2 anchor, Vector2 position)
	{
		this->position = Vector2Add(position, DeltaFromAnchor(GetBoundingBox(), anchor));
	}
	void Move(Vector2 delta) { position = Vector2Add(position, delta); }
	virtual Rectangle GetBoundingBox() const = 0;
	virtual void Update() = 0;
	virtual void Draw() const = 0;
	virtual bool CheckPointCollision(Vector2 point) const = 0;
};

class Pin : public Object
{
	constexpr static Vector2 extents = { 20, 40 };
	constexpr static Vector2 halfExt = { extents.x * 0.5f, extents.y * 0.5f };
	constexpr static Color color = GRAY;

public:
	Pin() = default;
	Pin(Vector2 position) : Object(position) {}
	Pin(Vector2 anchor, Vector2 position) : Object(anchor, position) {}
	~Pin() = default;
	Rectangle GetBoundingBox() const final
	{
		return { position.x - halfExt.x, position.y - halfExt.y, extents.x, extents.y };
	}
	void Update() final
	{
		// Todo
	}
	void Draw() const final
	{
		DrawRectangleRec(GetBoundingBox(), color);
	}
	bool CheckPointCollision(Vector2 point) const final
	{
		return CheckCollisionPointRec(point, GetBoundingBox());
	}
};

class Block : public Object
{
	constexpr static Vector2 extents = { 150, 100 };
	constexpr static Vector2 halfExt = { extents.x * 0.5f, extents.y * 0.5f };
	constexpr static Color color = GRAY;

public:
	Block() = default;
	Block(Vector2 position) : Object(position) {}
	Block(Vector2 anchor, Vector2 position) : Object(anchor, position) {}
	~Block() = default;
	Rectangle GetBoundingBox() const final
	{
		return { position.x - halfExt.x, position.y - halfExt.y, extents.x, extents.y };
	}
	void Update() final
	{
		// Todo
	}
	void Draw() const final
	{
		DrawRectangleRec(GetBoundingBox(), color);
	}
	bool CheckPointCollision(Vector2 point) const final
	{
		return CheckCollisionPointRec(point, GetBoundingBox());
	}
};

namespace Data
{
	// Data that gets serialized and reused between program instances
	class Serial
	{
	public:
		Serial()
		{
			std::ifstream file("session.ab");
			if (file)
			{
				// Todo
				file.close();
			}
		}
	};

	// Data that lasts across the lifetime of the program
	class Persistent
	{
	public:
		Persistent()
		{
			// Todo
		}
		std::vector<Object*> allObjects;
	};

	// Data that is cleaned and reset at the start of every frame
	class Frame
	{
	public:
		Frame()
		{
			// Todo
		}
	};
}

Data::Serial serial;
Data::Persistent persistent;
Data::Frame frame;

int main()
{
	InitWindow(1280, 720, "Assembly Block v0.0.1");

	SetTargetFPS(60);

	// Prep phase

	serial = Data::Serial();
	// Init serial
	{

	}

	persistent = Data::Persistent();
	// Init persistent
	{
		persistent.allObjects.push_back(new Pin({ 0,0 }, { 100, 0 }));
	}

	while (!WindowShouldClose())
	{
		frame = Data::Frame();

		// Sim phase
		{
			Vector2 mousePosition = GetMousePosition();

			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				for (Object* obj : persistent.allObjects)
				{
					obj->SetPosition(GetMousePosition());
				}
			}
			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
			{

			}
		}

		for (Object* obj : persistent.allObjects)
		{
			obj->Update();
		}

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(BLACK);

			for (Object* obj : persistent.allObjects)
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
