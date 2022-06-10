#include <raylib.h>
#include <fstream>
#include <unordered_set>
#include "Event.h"
#include "Engine.h"

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
		std::unordered_set<Engine::Object*> objects;
		size_t tickNumber = 0;
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

template<class T>
concept EngineObject = std::derived_from<T, Engine::Object>;

template<EngineObject T>
T* CreateObject(T&& base)
{
	T* object = new T(base);
	persistent.objects.insert(static_cast<Engine::Object*>(object));
	return object;
}
void Destroy(Engine::Object* instance)
{
	persistent.objects.erase(instance);
	delete instance;
}

int main()
{
	InitWindow(1280, 720, "Assembly Block v0.0.1");

	SetTargetFPS(60);

	// Prep phase

	serial = Data::Serial();

	persistent = Data::Persistent();

	Engine::Draggable* object = CreateObject(Engine::Draggable(Engine::Shapes::Rectangle2D(40, 40, 200, 300), true));
	object->SetActive(true);

	while (!WindowShouldClose())
	{
		frame = Data::Frame();

		// Sim phase
		{
			Vector2 mousePosition = GetMousePosition();
			Engine::InternalEvents::MouseEventArgs mouseArgs = { mousePosition };

			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
				Engine::InternalEvents::LeftMousePressEvent.Invoke(mouseArgs);

			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
				Engine::InternalEvents::LeftMouseReleaseEvent.Invoke(mouseArgs);
		}

		for (Engine::Object* it : persistent.objects)
		{
			it->Update();
		}

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(BLACK);

			for (Engine::Object* it : persistent.objects)
			{
				it->Draw();
			}
		}
		EndDrawing();
	}

	// Cleanup phase

	CloseWindow();

	return 0;
}
