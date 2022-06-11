#pragma once
#include "Data.h"
#include "Engine.h"
#include "Pin.h"
#include "Block.h"
#include "Node.h"

inline void Startup()
{
	decltype(Data::Persistent::allObjects)& objects = Data::Persistent::allObjects;

	BasicTransform trans;
	Instantiate<Node>(BasicTransform{ .position{ 600,400 } }, 2u);
}
