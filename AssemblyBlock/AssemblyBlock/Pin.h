#pragma once
#include "Engine.h"

class Pin : public ADDFocusable
{
public:
	static constexpr Vector2 pinExtents = { 20, 40 };
	static constexpr Color color_basic = DARKGRAY;
	static constexpr Color color_hovered = LIGHTGRAY;
	static constexpr Color color_focused = GRAY;

	Pin() = default;
	Pin(BasicTransform trans);
	~Pin() = default;

	void Update() final;
	void Draw() const final;
	inline const char* GetTypeName() const override { return "Pin"; }
};
