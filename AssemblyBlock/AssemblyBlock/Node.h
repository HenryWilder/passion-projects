#pragma once
#include "Block.h"
#include "Pin.h"

class Node : public Block
{
private:
	std::vector<ObjectTransform*> pins;

public:
	Node() = default;
	Node(BasicTransform trans, unsigned pinCount);
	~Node() = default;

	inline ObjectTransform* GetPin(size_t index) { return pins[index]; }

	void Update() override;
	void Draw() const override;
	inline const char* GetTypeName() const override { return "Node"; }
};
