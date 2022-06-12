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

	Node* node = Instantiate<Node>(BasicTransform{ .position{ 600,400 } }, 3);
	Pin* pin = Instantiate<Pin>(BasicTransform{ .position{ 100,0 } });
	Instantiate<Wire>(node->GetPin(0), pin); // Todo: Get pin from node
}
