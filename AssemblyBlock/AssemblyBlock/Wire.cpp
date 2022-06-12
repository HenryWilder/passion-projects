#include "Wire.h"

Wire::Wire(Object* start, Object* end)
{
	this->start = &start->transform;
	this->end = &end->transform;
}

Wire::Wire(ObjectTransform* start, ObjectTransform* end)
{
	this->start = start;
	this->end = end;
}

bool Wire::CheckPointComplexCollision(Vector2 point) const
{
	CheckCollisionPointLine(point, start->GetWorldPosition(), end->GetWorldPosition(), thickness);
	return false;
}

void Wire::Update()
{
	Focusable::Update();
}

void Wire::Draw() const
{
	_ASSERT_EXPR(start && end, L"Wires should be constructed with valid start and end before drawing");
	DrawLineEx(start->GetWorldPosition(), end->GetWorldPosition(), thickness,
		focused ? color_focused : (hovered ? color_hovered : color_normal));
}
