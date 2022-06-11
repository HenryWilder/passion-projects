#include "Pin.h"

void Pin::Update()
{
	Focusable::Update();
}
void Pin::Draw() const
{
	DrawRectangleRec(transform.WorldBounds(), (focused ? color_focused : (hovered ? color_hovered : color_basic)));
}
