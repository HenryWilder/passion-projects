#include <algorithm>
#include "Wire.h"

void Wire::ForwardUpdate()
{
	// Update bounds
	{
		Vector2 startPos = start->GetWorldPosition();
		Vector2 endPos = end->GetWorldPosition();
		Vector2 averagePoint = (startPos + endPos) * 0.5f;
		Vector2 channelDifference = (endPos - startPos);
		channelDifference.x = abs(channelDifference.x);
		channelDifference.y = abs(channelDifference.y);
		transform.SetWorldPosition(averagePoint);
		transform.SetExtents(channelDifference);
	}
	Focusable::ForwardUpdate();
}

void Wire::ReverseUpdate()
{
	Focusable::ReverseUpdate();
}

Wire::Wire(Object* start, Object* end) :
	Focusable(BasicTransform{ .parent{ &start->transform } })
{
	this->start = &start->transform;
	this->end = &end->transform;
}

Wire::Wire(ObjectTransform* start, ObjectTransform* end) :
	Focusable(BasicTransform{ .parent{ start } })
{
	this->start = start;
	this->end = end;
}

bool CheckCollisionLineCircle(Vector2 p1, Vector2 p2, Vector2 c, float r)
{
	// is either end INSIDE the circle?
	// if so, return true immediately
	float rSquared = r * r;
	bool inside1 = Vector2DistanceSqr(p1, c) <= rSquared;
	bool inside2 = Vector2DistanceSqr(p2, c) <= rSquared;
	if (inside1 || inside2) return true;

	// get length of the line
	float lenSqr = Vector2DistanceSqr(p1, p2);

	// get dot product of the line and circle
	float dot = Vector2DotProduct(c - p1, p2 - p1) / lenSqr;

	// find the closest point on the line
	Vector2 closest = Vector2Lerp(p1, p2, dot);

	DrawDebugCircle({ .point = closest, .radius = r });

	// is this point actually on the line segment?
	// if so keep going, but if not, return false
	auto [minx, maxx] = std::minmax(p1.x, p2.x);
	auto [miny, maxy] = std::minmax(p1.y, p2.y);
	Rectangle rec = { minx, miny, maxx - minx, maxy - miny };
	bool inBounds = CheckCollisionPointRec(closest, rec);
	if (!inBounds) return false;

	// get distance to closest point
	float distSqr = Vector2DistanceSqr(closest, c);

	bool colliding = distSqr <= rSquared;
	DrawDebugLine({ .start = p1, .end = p2, .thickness = 2, .color = (colliding ? GREEN : RED) });
	return colliding;
}

bool Wire::CheckPointComplexCollision(Vector2 point) const
{
	CheckCollisionLineCircle(start->GetWorldPosition(), end->GetWorldPosition(), point, thickness);
	return false;
}

void Wire::Draw() const
{
	_ASSERT_EXPR(start && end, L"Wires should be constructed with valid start and end before drawing");
	DrawLineEx(start->GetWorldPosition(), end->GetWorldPosition(), thickness,
		focused ? color_focused : (hovered ? color_hovered : color_normal));
}
