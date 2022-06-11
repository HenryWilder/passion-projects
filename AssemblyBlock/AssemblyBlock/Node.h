#pragma once
#include "Block.h"
#include "Pin.h"

class Node : public Block
{
public:
	Node() = default;
	Node(BasicTransform trans, unsigned pinCount);
	~Node() = default;

	void Update() override;
	void Draw() const override;
};
