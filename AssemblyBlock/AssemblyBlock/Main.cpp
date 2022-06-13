#include <unordered_set>
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
		for (size_t i = Data::Persistent::allObjects.size(); i > 0; --i)
		{
			Object* what = Data::Persistent::allObjects[i - 1];
			if (what->_rbf) what->ReverseUpdate();
		}
		for (size_t i = 0; i < Data::Persistent::allObjects.size(); ++i)
		{
			Object* what = Data::Persistent::allObjects[i];
			what->ForwardUpdate();
		}
		for (size_t i = Data::Persistent::allObjects.size(); i > 0; --i)
		{
			Object* what = Data::Persistent::allObjects[i - 1];
			if (!what->_rbf) what->ReverseUpdate();
		}

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			for (Object* obj : Data::Persistent::allObjects)
			{
				obj->Draw();
			}
			DrawDebugPoint({ .point = Data::Frame::cursor, .color = BLUE });
			_DrawDebug();
		}
		EndDrawing();
	}

	// Cleanup phase

	CloseWindow();

	return 0;
}
