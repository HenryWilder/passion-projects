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

	ObjectTransform& nodeTrans = Instantiate<Node>(BasicTransform{ .position{ 600,400 } }, 3);
	ObjectTransform& pinTrans = Instantiate<Pin>(BasicTransform{ .position{ 100,0 } });
	Instantiate<Wire>(nodeTrans.ob, &pinTrans); // Todo: Get pin from node
}
