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
	Pin(BasicTransform trans, bool isSquarePin = false);
	~Pin() = default;

	void Update() final;
	void Draw() const final;
#if _DEBUG
	virtual void DrawDebug() const override;
#endif
	inline const char* GetTypeName() const override { return "Pin"; }
};
