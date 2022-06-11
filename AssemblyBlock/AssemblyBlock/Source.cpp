#include "Engine.h"
#include "Pin.h"
#include "Block.h"

int main()
{
	InitWindow(1280, 720, "Assembly Block v0.0.1");

	SetTargetFPS(60);

	// Prep phase

	Data::Serial::Load();

	Data::Persistent::Init();
	// Init persistent
	{
		decltype(Data::Persistent::allObjects)& objects = Data::Persistent::allObjects;
		constexpr float pinWidth = Pin::pinExtents.x;
		constexpr float pinHeight = Pin::pinExtents.y;
		constexpr float blockWidth = Block::blockExtents.x;
		constexpr float blockHeight = Block::blockExtents.y;

		{
			Instantiate<Pin>({ 100, 0 }, { 0, 1 });
		}
		{
			Instantiate<Block>({ 400, 0 }, { 0, 1 });
		}
		{
			ObjectTransform& pin = Instantiate<Pin>();
			pin.SetParent(objects[1]->transform);
			pin.SetLocalPosition({ blockWidth / 2, 0 }, { 0.5, 0.5f });
		}
		{
			ObjectTransform& pin = Instantiate<Pin>();
			pin.SetParent(objects[1]->transform);
			pin.SetLocalPosition({ blockWidth / 2 + pinWidth * 2, 0 }, { 0.5, 0.5f });
		}
		{
			ObjectTransform& pin = Instantiate<Pin>();
			pin.SetParent(objects[1]->transform);
			pin.SetLocalPosition({ blockWidth / 2 + pinWidth * 4, 0 }, { 0.5, 0.5f });
		}
	}

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
