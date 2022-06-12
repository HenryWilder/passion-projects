#include "Node.h"

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

void Node::Update()
{
	Block::Update();
}

void Node::Draw() const
{
	Block::Draw();
}

#if _DEBUG
void Node::DrawDebug() const
{
	// Todo
}
#endif
