#include <unordered_set>
#include "Data.h"
#include "Engine.h"
#include "Startup.h"

std::queue<Object*> reverseAfterForewardObjects;
std::unordered_map<Object*, std::vector<Object*>> unEvaluatedDependents;
std::unordered_set<Object*> beenEvaluated;

template<bool isForwardUpdate>
void TryEvaluate(Object* object)
{
	do {
		const std::vector<Object*>& dependencies = object->DependentOn();
		if (dependencies.empty()) break; // Good to evaluate
		for (Object* dep : dependencies)
		{
			if (!beenEvaluated.contains(dep))
			{
				if (unEvaluatedDependents.contains(dep))
					unEvaluatedDependents.at(dep).push_back(object);
				else
					unEvaluatedDependents.insert({ dep, { object } });
				return;
			}
		}
	} while (false);

	// Evaluate
	if constexpr (isForwardUpdate) object->ForwardUpdate();
	else						   object->ReverseUpdate();
	beenEvaluated.insert(object);

	// Evaluate everything dependent on this
	if (unEvaluatedDependents.contains(object))
	{
		for (Object* dependent : unEvaluatedDependents.at(object))
		{
			TryEvaluate<isForwardUpdate>(dependent);
		}
		unEvaluatedDependents.erase(object);
	}
}

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
			if (what->_rbf)
				TryEvaluate<false>(what);
			else
				reverseAfterForewardObjects.push(what);
		}
		for (size_t i = 0; i < Data::Persistent::allObjects.size(); ++i)
		{
			Object* what = Data::Persistent::allObjects[i];
			what->ForwardUpdate();
			TryEvaluate<false>(what);
		}
		while (!reverseAfterForewardObjects.empty())
		{
			Object* what = reverseAfterForewardObjects.front();
			TryEvaluate<false>(what);
			reverseAfterForewardObjects.pop();
		}
		beenEvaluated.clear();
		unEvaluatedDependents.clear();

		// Draw phase
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			for (Object* obj : Data::Persistent::allObjects)
			{
				obj->Draw();
			}
			DrawDebugPoint({ .point{ Data::Frame::cursor } });
			_DrawDebug();
		}
		EndDrawing();
	}

	// Cleanup phase

	CloseWindow();

	return 0;
}
