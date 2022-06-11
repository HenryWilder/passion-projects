#pragma once
#include <raylib.h>
#include <vector>
#include "Engine.h"

class Object;

namespace Data
{
	// Data that gets serialized and reused between program instances
	namespace Serial
	{
		// Todo: put serializable data here

		void Load();
		void Save();
	}

	// Data that lasts across the lifetime of the program
	namespace Persistent
	{
		extern std::vector<Object*> allObjects;

		void Init();
	}

	// Data that is cleaned and reset at the start of every frame
	namespace Frame
	{
		extern Vector2 cursor;

		void Init();
	}
}
