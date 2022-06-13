#pragma once
#include "Data.h"
#include "Engine.h"
#include "Pin.h"
#include "Block.h"
#include "Wire.h"
#include "Node.h"

inline void Startup()
{
	decltype(Data::Persistent::allObjects)& objects = Data::Persistent::allObjects;

	for (unsigned i = 0; i < 16; ++i)
	{
		Instantiate<MemoryPin>(BasicTransform{ .position{ 100 + i * MemoryPin::pinExtents.x * 1.5f, 0 } });
	}
	Node* node = Instantiate<Node>(BasicTransform{ .position{ 600,400 } }, 3, "Text");
	Instantiate<Wire>(node->GetPin(0), objects[0]); // Todo: Get pin from node
}
