#pragma once
#include "Frame.h"

// A text output for displaying warnings, errors, assertions, and letting the user know when something needs attention
class Console : public Frame
{
public:
	void TickActive() final;
	void TickPassive() final;

	void Draw() const final;

	inline const char* GetName() const final { return "Console"; }
};
