#include <algorithm>
#include <map>
#include <stack>
#include "Engine.h"

#if _DEBUG
std::queue<Debug::DebugShape*> Debug::_debugDrawQueue;

void _DrawDebug()
{
	while (!Debug::_debugDrawQueue.empty())
	{
		auto next = Debug::_debugDrawQueue.front();
		Debug::_debugDrawQueue.pop();
		next->Draw();
		delete next;
	}
}
#endif


Vector2 LocalFromAnchor(Vector2 extents, Vector2 anchor)
{
	return extents * anchor;
}

Vector2 AnchorFromLocal(Vector2 extents, Vector2 localPos)
{
	Vector2 ret;
	ret.x = ((extents.x == 0) ? (0) : (localPos.x / extents.x));
	ret.y = ((extents.y == 0) ? (0) : (localPos.y / extents.y));
	return ret;
}

Vector2 RectanglePosition(Rectangle rec)
{
	return { rec.x, rec.y };
}

Vector2 RectangleExtents(Rectangle rec)
{
	return { rec.width, rec.height };
}

void SetRectanglePosition(Rectangle& rec, Vector2 pos)
{
	rec.x = pos.x;
	rec.y = pos.y;
}

void AddRectanglePosition(Rectangle& rec, Vector2 offset)
{
	rec.x += offset.x;
	rec.y += offset.y;
}

void SetRectangleExtents(Rectangle& rec, Vector2 ext)
{
	rec.width = ext.x;
	rec.height = ext.y;
}
