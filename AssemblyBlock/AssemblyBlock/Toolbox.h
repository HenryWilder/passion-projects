#pragma once
#include "Frame.h"

// A box for tools and actions
class Toolbox : public Frame
{
public:
	void TickActive() final;
	void TickPassive() final;

	void Draw() const final;

	inline const char* GetName() const final { return "Toolbox"; }
};
