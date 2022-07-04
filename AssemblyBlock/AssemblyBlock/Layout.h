#pragma once
#include <deque>
#include "Panel.h"

namespace Layout
{
	// A subdividable section of the screen which can contain children
	class Block
	{
	private:
		Block* parent = nullptr;
		Panel* content = nullptr;
		// Percent of parent block's size this block takes up.
		// If no parent, this is actual size.
		float size = 1.0f;
		// In order of left-to-right or top-to-bottom
		std::deque<Block*> children;

	public:
		void AddChild(size_t at, Block* childBlock);
		Block* ExtractChild(size_t index);
		float GetScreenSize() const;
	};
	Block mainBlock;
}
