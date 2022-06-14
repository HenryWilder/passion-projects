#include "Data.h"
#include "Engine.h"

Event<Vector2> MouseMovedEvent;

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
		Vector2 cursorPrev;
		Vector2 cursor = { NAN, NAN };
		Vector2 cursorDelta;
		bool cursorMoved;

		void Init()
		{
			cursorPrev = cursor;
			cursor = GetMousePosition();
			cursorDelta = cursor - cursorPrev;
			cursorMoved =
				cursor.x != cursorPrev.x ||
				cursor.y != cursorPrev.y;

			if (cursorMoved)
				MouseMovedEvent.Invoke(cursor);
		}
	}
}
