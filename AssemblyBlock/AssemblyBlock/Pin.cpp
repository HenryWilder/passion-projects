#include "Engine.h"
#include "Pin.h"

Pin::Pin(ObjectTransform trans) : ADDFocusable(trans) { transform.SetExtents(pinExtents); }

void Pin::Update()
{
	ADDFocusable::Update();
}
void Pin::Draw() const
{
	DrawRectangleRec(transform.WorldBounds(), (focused ? color_focused : (hovered ? color_hovered : color_basic)));
}
