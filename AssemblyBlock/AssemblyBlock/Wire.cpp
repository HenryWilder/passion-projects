#include "Wire.h"

Wire::Wire(Object* start, Object* end)
{
	this->start = &start->transform;
	this->end = &end->transform;
}

Wire::Wire(ObjectTransform* start, ObjectTransform* end) :
	Focusable(BasicTransform{})
{
	this->start = start;
	this->end = end;
}

bool Wire::CheckPointComplexCollision(Vector2 point) const
{
	CheckCollisionPointLine(point, start->GetWorldPosition(), end->GetWorldPosition(), (int)thickness);
	return false;
}

void Wire::Update()
{
	Focusable::Update();
	Vector2 averagePoint = (start->GetWorldPosition() + end->GetWorldPosition()) * 0.5f;
	Vector2 channelDifference = (end->GetWorldPosition() - start->GetWorldPosition());
	channelDifference.x = abs(channelDifference.x);
	channelDifference.y = abs(channelDifference.y);
	transform.SetWorldPosition(averagePoint);
	transform.SetExtents(channelDifference);
}

void Wire::Draw() const
{
	_ASSERT_EXPR(start && end, L"Wires should be constructed with valid start and end before drawing");
	DrawLineEx(start->GetWorldPosition(), end->GetWorldPosition(), thickness,
		focused ? color_focused : (hovered ? color_hovered : color_normal));
}

#if _DEBUG
void Wire::DrawDebug() const
{
	Rectangle rec = transform.WorldBounds();
	DrawRectangleLines(rec.x, rec.y, rec.width, rec.height, MAGENTA);
}
#endif
