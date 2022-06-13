#pragma once
#include "Engine.h"

class Block : public Draggable
{
protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	static constexpr Vector2 blockExtents = { 150, 100 };
	static constexpr Color color_basic = DARKGRAY;
	static constexpr Color color_hovered = LIGHTGRAY;
	static constexpr Color color_dragged = GRAY;

	Block() = default;
	Block(BasicTransform trans);
	~Block() = default;

	void Draw() const override;
	inline const char* GetTypeName() const override { return "Block"; }
};
