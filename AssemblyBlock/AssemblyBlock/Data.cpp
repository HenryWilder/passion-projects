#include "Engine.h"

namespace Data
{
	namespace Serial
	{
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

	namespace Persistent
	{
		std::vector<Object*> allObjects;

		void Init()
		{
			// Todo
		}
	}

	namespace Frame
	{
		Vector2 cursor;

		void Init()
		{
			cursor = GetMousePosition();
		}
	}
}
