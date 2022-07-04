#include "Layout.h"

namespace Layout
{
	void Block::AddChild(size_t at, Block* child)
	{
		_ASSERT_EXPR(child->parent == nullptr, L"Attempting to add child to block while child is still parented to another block. Make sure other parent is aware of this.");
		_ASSERT(at <= children.size());
		_ASSERT(child != nullptr);
		children.insert(children.begin() + at, child);
		child->parent = this;
	}

	Block* Block::ExtractChild(size_t index)
	{
		_ASSERT(index < children.size());
		Block* child = children[index];
		children.erase(children.begin() + index);
		child->parent = nullptr;
		return child;
	}

	float Block::GetScreenSize() const
	{
		float pixelSize = 1.0f;
		const Block* ancestor = this;
		while (ancestor)
		{
			pixelSize *= ancestor->size;
			ancestor = ancestor->parent;
		}
		return pixelSize;
	}
}
