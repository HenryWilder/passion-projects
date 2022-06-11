#pragma once
#include "Engine.h"

class Block : public Draggable
{
public:
	static constexpr Vector2 blockExtents = { 150, 100 };
	static constexpr Color color_basic = DARKGRAY;
	static constexpr Color color_hovered = LIGHTGRAY;
	static constexpr Color color_dragged = GRAY;

	Block() = default;
	Block(ObjectTransform trans) : Draggable(trans) { transform.SetExtents(blockExtents); }
	~Block() = default;

	void Update() final
	{
		Draggable::Update();
	}
	void Draw() const final
	{
		DrawRectangleRec(transform.WorldBounds(), (beingDragged ? color_dragged : (hovered ? color_hovered : color_basic)));
	}
};

