#pragma once
#include "Frame.h"

// A box for tools and actions
class Toolbox : public Frame
{
public:
	void TickActive() final
	{
		// Todo
	}
	void TickPassive() final
	{
		// Todo
	}

	void Draw() const final
	{
		// Todo
	}

	const char* GetName() const final { return "Toolbox"; }
};
