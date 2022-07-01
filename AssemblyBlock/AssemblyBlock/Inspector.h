#pragma once
#include "Frame.h"

// A framel displaying informations and options for the selection
class Inspector : public Frame
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

	const char* GetName() const final { return "Inspector"; }
};
