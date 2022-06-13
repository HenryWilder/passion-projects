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

Pin::Pin(BasicTransform trans, bool isSquarePin) : ADDFocusable(trans)
{
	Vector2 ext = pinExtents;
	if (isSquarePin) ext.y = ext.x;
	transform.SetExtents(ext);
}

void Pin::Draw() const
{
	DrawRectangleRec(transform.WorldBounds(), (focused ? color_focused : (hovered ? color_hovered : color_basic)));
}
