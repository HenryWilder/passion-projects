#include "Block.h"

Block::Block(BasicTransform trans) : Draggable(trans) { transform.SetExtents(blockExtents); }

void Block::Update()
{
	Draggable::Update();
}
void Block::Draw() const
{
	DrawRectangleRec(transform.WorldBounds(), (beingDragged ? color_dragged : (hovered ? color_hovered : color_basic)));
}

#if _DEBUG
void Block::DrawDebug() const
{
	// Todo
}
#endif
