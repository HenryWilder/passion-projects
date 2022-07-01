#pragma once
#include "Frame.h"

// A framel displaying informations and options for the selection
class Inspector : public Frame
{
public:
	void TickActive() final;
	void TickPassive() final;

	void Draw() const final;

	inline const char* GetName() const final { return "Inspector"; }
};
