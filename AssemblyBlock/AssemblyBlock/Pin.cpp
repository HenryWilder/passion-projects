#include "Engine.h"
#include "Pin.h"

void Pin::ForwardUpdate()
{
	ADDFocusable::ForwardUpdate();
}

void Pin::ReverseUpdate()
{
	ADDFocusable::ReverseUpdate();
}

Pin::Pin(BasicTransform trans) : ADDFocusable(trans) {}

void Pin::Draw() const
{
	DrawRectangleRec(transform.WorldBounds(), (focused ? color_focused : (hovered ? color_hovered : color_basic)));
}

std::tuple<Vector2, Vector2, Vector2> ExecutionPin::TriangleFromBounds(Rectangle rec)
{
	return { { rec.x, rec.y }, { rec.x, rec.y + rec.height }, { rec.x + rec.width, rec.y + rec.height * 0.5f } };
}

void ExecutionPin::ForwardUpdate()
{
	Pin::ForwardUpdate();
}

void ExecutionPin::ReverseUpdate()
{
	Pin::ReverseUpdate();
}

ExecutionPin::ExecutionPin(BasicTransform trans) : Pin(trans)
{
	transform.SetExtents(pinExtents);
}

bool ExecutionPin::CheckPointComplexCollision(Vector2 point) const
{
	auto [a, b, c] = TriangleFromBounds(transform.WorldBounds());
	return CheckCollisionPointTriangle(point, a,b,c);
}

void ExecutionPin::Draw() const
{
	auto [a,b,c] = TriangleFromBounds(transform.WorldBounds());
	DrawTriangle(a,b,c, (focused ? color_focused : (hovered ? color_hovered : color_basic)));
}

void MemoryPin::ForwardUpdate()
{
	Pin::ForwardUpdate();
}

void MemoryPin::ReverseUpdate()
{
	Pin::ReverseUpdate();
}

MemoryPin::MemoryPin(BasicTransform trans) : Pin(trans)
{
	transform.SetExtents(pinExtents);
}

void MemoryIOPin::ForwardUpdate()
{
	Pin::ForwardUpdate();
}

void MemoryIOPin::ReverseUpdate()
{
	Pin::ReverseUpdate();
}

MemoryIOPin::MemoryIOPin(BasicTransform trans) : Pin(trans)
{
	transform.SetExtents(pinExtents);
}
