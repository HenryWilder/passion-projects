#pragma once
#include "Engine.h"

class Wire;

class Pin : public ADDFocusable
{
protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;
	bool inDragMode = false;
	Pin* createdPin = nullptr;
	Wire* createdwire = nullptr;

public:
	static constexpr Color color_basic = DARKGRAY;
	static constexpr Color color_hovered = LIGHTGRAY;
	static constexpr Color color_focused = GRAY;

	Pin() = default;
	Pin(BasicTransform trans, bool instantlyDragging = false);
	~Pin() = default;

	void Draw() const override;
	inline const char* GetTypeName() const override { return "Pin"; }
};


// Pin representing a location in RAM
class MemoryPin : public Pin
{
protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

	void OnFocus() final;
	void OnLoseFocus() final;

public:
	static constexpr Vector2 pinExtents = { 20, 40 };

	MemoryPin() = default;
	MemoryPin(BasicTransform trans, bool instantlyDragging = false);
	~MemoryPin() = default;

	inline const char* GetTypeName() const override { return "Memory pin"; }
};

// Pin wanting to access a location in memory
class MemoryIOPin : public Pin
{
protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	static constexpr Vector2 pinExtents = { 20, 20 };

	MemoryIOPin() = default;
	MemoryIOPin(BasicTransform trans, bool instantlyDragging = false);
	~MemoryIOPin() = default;

	inline const char* GetTypeName() const override { return "Memory IO pin"; }
};

// Pin representing the order of execution
class ExecutionPin : public Pin
{
private:
	static std::tuple<Vector2, Vector2, Vector2> TriangleFromBounds(Rectangle rec);

protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	static constexpr Vector2 pinExtents = { 20, 20 };

	ExecutionPin() = default;
	ExecutionPin(BasicTransform trans, bool instantlyDragging = false);
	~ExecutionPin() = default;

	virtual bool CheckPointComplexCollision(Vector2 point) const;
	virtual constexpr bool IsComplexCollisionDifferentFromSimpleCollision() const { return true; }

	void Draw() const final;
	inline const char* GetTypeName() const override { return "Execution pin"; }
};
