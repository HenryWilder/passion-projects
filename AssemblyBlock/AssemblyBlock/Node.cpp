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
		.position{MemoryIOPin::pinExtents + Vector2{0,4}} }, text, textColor);
	this->text->transform.SetExtents({ 0,20 });

	{
		pins.reserve(pinCount);
		constexpr float xStart = blockExtents.x * 0.25f;
		constexpr float xIncrement = MemoryIOPin::pinExtents.x * 2;
		float x = xStart;
		constexpr float y = MemoryIOPin::pinExtents.y * -0.5f;
		for (unsigned i = 0; i < pinCount; ++i)
		{
			MemoryIOPin* pin = Instantiate<MemoryIOPin>(BasicTransform{ .parent = &transform, .position{ x, y } });
			pins.push_back(pin);
			x += xIncrement;
		}
	}
	{
		exPins.reserve(2);
		float xOffset = ExecutionPin::pinExtents.x * 0.35f;
		float y = blockExtents.y * 0.5f;
		exPins.push_back(Instantiate<ExecutionPin>(BasicTransform{ .parent = &transform, .position{ -xOffset, y } }));
		exPins.push_back(Instantiate<ExecutionPin>(BasicTransform{ .parent = &transform, .position{ blockExtents.x - xOffset, y } }));
	}
}

void Node::Draw() const
{
	Block::Draw();
}
