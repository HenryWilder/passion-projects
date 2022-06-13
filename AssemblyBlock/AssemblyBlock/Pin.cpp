#include "Engine.h"
#include "Wire.h"
#include "Pin.h"

void Pin::ForwardUpdate()
{
	if (inDragMode)
		transform.SetWorldPosition(Data::Frame::cursor);
	ADDFocusable::ForwardUpdate();
}

void Pin::ReverseUpdate()
{
	ADDFocusable::ReverseUpdate();
}

Pin::Pin(BasicTransform trans, bool instantlyDragging) : ADDFocusable(trans), inDragMode(instantlyDragging) {}

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

ExecutionPin::ExecutionPin(BasicTransform trans, bool instantlyDragging) : Pin(trans, instantlyDragging)
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

void MemoryPin::OnFocus()
{
	createdPin = Instantiate<MemoryPin>(BasicTransform{ .position = Data::Frame::cursor }, true);
	createdwire = Instantiate<Wire>(this, createdPin);
}

void MemoryPin::OnLoseFocus()
{
	if (createdPin)
	{
		Destroy(createdPin);
		createdPin = nullptr;
	}
	if (createdwire)
	{
		Destroy(createdwire);
		createdwire = nullptr;
	}
}

MemoryPin::MemoryPin(BasicTransform trans, bool instantlyDragging) : Pin(trans, instantlyDragging)
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

MemoryIOPin::MemoryIOPin(BasicTransform trans, bool instantlyDragging) : Pin(trans, instantlyDragging)
{
	transform.SetExtents(pinExtents);
}
