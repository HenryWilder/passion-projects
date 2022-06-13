#pragma once
#include "Block.h"
#include "Pin.h"

class Node : public Block
{
private:
	std::vector<MemoryIOPin*> pins;
	std::vector<ExecutionPin*> exPins;
	TextRenderer* text;
	static constexpr Color textColor = WHITE;

protected:
	void ForwardUpdate() override;
	void ReverseUpdate() override;

public:
	Node() = default;
	Node(BasicTransform trans, unsigned pinCount, const std::string& text);

	~Node() = default;

	inline MemoryIOPin* GetPin(size_t index) { return pins[index]; }

	void Draw() const override;
	inline const char* GetTypeName() const override { return "Node"; }
};
