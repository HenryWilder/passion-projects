#pragma once
#include "Block.h"
#include "Pin.h"

class Node : public Block
{
private:
	std::vector<Pin*> pins;

public:
	Node() = default;
	Node(BasicTransform trans, unsigned pinCount);
	~Node() = default;

	inline Pin* GetPin(size_t index) { return pins[index]; }

	void Update() override;
	void Draw() const override;
#if _DEBUG
	virtual void DrawDebug() const override;
#endif
	inline const char* GetTypeName() const override { return "Node"; }
};
