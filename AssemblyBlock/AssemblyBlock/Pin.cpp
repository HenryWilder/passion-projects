#include "Engine.h"
#include "Pin.h"

Pin::Pin(BasicTransform trans, bool isSquarePin) : ADDFocusable(trans)
{
	Vector2 ext = pinExtents;
	if (isSquarePin) ext.y = ext.x;
	transform.SetExtents(ext);
}

void Pin::Update()
{
	ADDFocusable::Update();
}
void Pin::Draw() const
{
	DrawRectangleRec(transform.WorldBounds(), (focused ? color_focused : (hovered ? color_hovered : color_basic)));
}

#if _DEBUG
void Pin::DrawDebug() const
{
	// Todo
}
#endif
