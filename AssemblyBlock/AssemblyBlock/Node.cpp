#include "Node.h"

Node::Node(BasicTransform trans, unsigned pinCount) : Block(trans)
{
	float x = blockExtents.x / 4;
	for (int i = 0; i < pinCount; ++i)
	{
		ObjectTransform& pinTransform = Instantiate<Pin>(BasicTransform{ .parent{ &transform }, .position{ x, 0 } });
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
	for (ObjectTransform* child : transform)
	{
		DrawLineV(transform.GetWorldPosition(), child->GetWorldPosition(), MAGENTA);
	}
}
