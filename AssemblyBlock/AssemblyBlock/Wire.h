#pragma once
#include "Engine.h"

// Note: wire transform is not helpful!!
class Wire : public Focusable
{
private:
	void SetupDependancies();

protected:
	ObjectTransform* start = nullptr;
	ObjectTransform* end = nullptr;

	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	static constexpr Color color_normal = GRAY;
	static constexpr Color color_hovered = LIGHTGRAY;
	static constexpr Color color_focused = SKYBLUE;
	static constexpr float thickness = 4.0f;

	Wire() = default;
	Wire(Object* start, Object* end);
	Wire(ObjectTransform* start, ObjectTransform* end);
	~Wire() = default;

	virtual bool CheckPointComplexCollision(Vector2 point) const;
	virtual constexpr bool IsComplexCollisionDifferentFromSimpleCollision() const { return true; }

	void Draw() const override;
	inline const char* GetTypeName() const override { return "Wire"; }
};
