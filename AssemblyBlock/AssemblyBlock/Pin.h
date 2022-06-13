#pragma once
#include "Engine.h"

class Pin : public ADDFocusable
{
protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	static constexpr Vector2 pinExtents = { 20, 40 };
	static constexpr Color color_basic = DARKGRAY;
	static constexpr Color color_hovered = LIGHTGRAY;
	static constexpr Color color_focused = GRAY;

	Pin() = default;
	Pin(BasicTransform trans, bool isSquarePin = false);
	~Pin() = default;

	void Draw() const final;
	inline const char* GetTypeName() const override { return "Pin"; }
};
