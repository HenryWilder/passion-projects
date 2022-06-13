#include "Node.h"

void Node::ForwardUpdate()
{
	Block::ForwardUpdate();
}

void Node::ReverseUpdate()
{
	Block::ReverseUpdate();
}

Node::Node(BasicTransform trans, unsigned pinCount, const std::string& text) :
	Block(trans)
{
	transform.SetPivot({0,0});

	this->text = Instantiate<TextRenderer>(
		BasicTransform{ .parent = &transform, .pivot{ 0,0 },
		.position{Pin::pinExtents + Vector2{0,4}} }, text, textColor);
	this->text->transform.SetExtents({ 0,20 });

	pins.reserve(pinCount);
	float x = blockExtents.x * 0.25f;
	float y = 0; // -(blockExtents.y + Pin::pinExtents.y) * 0.5f;
	for (unsigned i = 0; i < pinCount; ++i)
	{
		Pin* pin = Instantiate<Pin>(BasicTransform{ .parent = &transform, .position{ x, y } });
		pins.push_back(pin);
		x += Pin::pinExtents.x * 2;
	}
}

void Node::Draw() const
{
	Block::Draw();
}
