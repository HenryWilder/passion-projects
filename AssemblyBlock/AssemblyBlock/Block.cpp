#include "Block.h"

void Block::ForwardUpdate()
{
	Draggable::ForwardUpdate();
}

void Block::ReverseUpdate()
{
	Draggable::ReverseUpdate();
}

Block::Block(BasicTransform trans) : Draggable(trans) { transform.SetExtents(blockExtents); }

void Block::Draw() const
{
	DrawRectangleRec(transform.WorldBounds(), (beingDragged ? color_dragged : (hovered ? color_hovered : color_basic)));
}
