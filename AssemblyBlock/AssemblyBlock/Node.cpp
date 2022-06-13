#include "Node.h"

void Node::ForwardUpdate()
{
	Block::ForwardUpdate();
}

void Node::ReverseUpdate()
{
	Block::ReverseUpdate();
}

Node::Node(BasicTransform trans, unsigned pinCount) :
	Block(trans)
{
	pins.reserve(pinCount);
	float x = blockExtents.x / 4;
	for (unsigned i = 0; i < pinCount; ++i)
	{
		Pin* pin = Instantiate<Pin>(BasicTransform{ .parent{ &transform }, .position{ x, 0 } });
		pins.push_back(pin);
		x += Pin::pinExtents.x * 2;
	}
}

void Node::Draw() const
{
	Block::Draw();
}
