#include "Data.h"
#include "Engine.h"
#include "Startup.h"

int main()
{
	InitWindow(1280, 720, "Assembly Block v0.0.1");

	SetTargetFPS(60);

	// Prep phase

	Data::Serial::Load();

	Data::Persistent::Init();
	Startup();
	SortObjects();

	while (!WindowShouldClose())
	{
		Data::Frame::Init();

		// Sim phase
		for (Object* obj : Data::Persistent::allObjects)
		{
			obj->Update();
		}

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			for (Object* obj : Data::Persistent::allObjects)
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
