#include <raylib.h>
#include <thread>
#include "IVec.h"
#include "Graph.h"
#include "Window.h"
#include "Tab.h"

Tab::Tab(Window* owner, const char* name) :
	owningWindow(owner),
	camera{ .offset{ 0,0 }, .target{ 0,0 }, .rotation{ 0 }, .zoom{ 1 } },
	graph(new Graph(this, name)) {}

Tab::~Tab()
{
	delete graph;
}

IRect* Tab::GetLastSelectionRec()
{
	if (SelectionRectExists())
		return &selectionRecs.back();
	return nullptr;
}

const IRect* Tab::GetLastSelectionRecConst() const
{
	if (SelectionRectExists())
		return &selectionRecs.back();
	return nullptr;
}

void Tab::CreateSelectionRec(IRect rec)
{
	if (rec.w > 0 && rec.h > 0)
		selectionRecs.push_back(rec);
}

bool Tab::IsSelectionBridgeable() const
{
	// Must be exactly two rectangles
	if (SelectionRectCount() != 2)
		return false;
	
	// Must have same number of nodes in each selection rectangle
	size_t rec1Nodes = 0;
	size_t rec2Nodes = 0;
	for (Node* node : selection)
	{
		if (InBoundingBox(selectionRecs[0], node->GetPosition()))
			++rec1Nodes;
		else
		{
			_ASSERT_EXPR(InBoundingBox(selectionRecs[1], node->GetPosition()), L"Node in selection was in neither selection rectangle");
			++rec2Nodes;
		}
	}
	return rec1Nodes == rec2Nodes || ((rec1Nodes == 1 || rec2Nodes == 1) && selection.size() > 1);
}
void Tab::BridgeSelection(ElbowConfig elbow)
{
	_ASSERT_EXPR(IsSelectionBridgeable(), L"Selection is not bridgable");

	std::vector<Node*> rec1Nodes;
	std::vector<Node*> rec2Nodes;
	for (Node* node : selection)
	{
		if (InBoundingBox(selectionRecs[0], node->GetPosition()))
			rec1Nodes.push_back(node);
		else
		{
			_ASSERT_EXPR(InBoundingBox(selectionRecs[1], node->GetPosition()), L"Node in selection was in neither selection rectangle");
			rec2Nodes.push_back(node);
		}
	}

	auto sortNodes = [](std::vector<Node*>& nodeVec) {
		std::sort(nodeVec.begin(), nodeVec.end(),
			[](Node* a, Node* b)
			{
				return (a->GetX() != b->GetX() ? a->GetX() < b->GetX() : a->GetY() < b->GetY());
			});
	};

	if (rec1Nodes.size() == 1 && rec2Nodes.size() == 1)
	{
		graph->CreateWire(rec1Nodes[0], rec2Nodes[0], elbow);
	}
	else if (rec1Nodes.size() == 1)
	{
		sortNodes(rec2Nodes);
		for (size_t i = 0; i < rec2Nodes.size(); ++i)
		{
			graph->CreateWire(rec1Nodes[0], rec2Nodes[i], elbow);
		}
	}
	else if (rec2Nodes.size() == 1)
	{
		sortNodes(rec1Nodes);
		for (size_t i = 0; i < rec1Nodes.size(); ++i)
		{
			graph->CreateWire(rec1Nodes[i], rec2Nodes[0], elbow);
		}
	}
	else
	{
		{
			std::thread a(sortNodes, std::ref(rec1Nodes));
			std::thread b(sortNodes, std::ref(rec2Nodes));
			a.join();
			b.join();
		}

		for (size_t i = 0; i < rec1Nodes.size(); ++i)
		{
			graph->CreateWire(rec1Nodes[i], rec2Nodes[i], elbow);
		}
	}
}

void Tab::DrawBridgePreview(ElbowConfig elbow, Color color) const
{
	_ASSERT_EXPR(IsSelectionBridgeable(), L"Selection is not bridgable");

	std::vector<Node*> rec1Nodes;
	std::vector<Node*> rec2Nodes;
	for (Node* node : selection)
	{
		if (InBoundingBox(selectionRecs[0], node->GetPosition()))
			rec1Nodes.push_back(node);
		else
		{
			_ASSERT_EXPR(InBoundingBox(selectionRecs[1], node->GetPosition()), L"Node in selection was in neither selection rectangle");
			rec2Nodes.push_back(node);
		}
	}

	auto sortNodes = [](std::vector<Node*>& nodeVec) {
		std::sort(nodeVec.begin(), nodeVec.end(),
			[](Node* a, Node* b)
			{
				return (a->GetX() != b->GetX() ? a->GetX() < b->GetX() : a->GetY() < b->GetY());
			});
	};

	if (rec1Nodes.size() == 1 && rec2Nodes.size() == 1)
	{
		Wire(rec1Nodes.back(), rec2Nodes.back(), elbow).Draw(color);
	}
	else if (rec1Nodes.size() == 1)
	{
		sortNodes(rec2Nodes);
		for (size_t i = 0; i < rec2Nodes.size(); ++i)
		{
			// Holy cow this is so much nicer! Why aren't I using this everywhere?!
			Wire(rec1Nodes.back(), rec2Nodes[i], elbow).Draw(color);
		}
	}
	else if (rec2Nodes.size() == 1)
	{
		sortNodes(rec1Nodes);
		for (size_t i = 0; i < rec1Nodes.size(); ++i)
		{
			Wire(rec1Nodes[i], rec2Nodes.back(), elbow).Draw(color);
		}
	}
	else
	{
		{
			std::thread a(sortNodes, std::ref(rec1Nodes));
			std::thread b(sortNodes, std::ref(rec2Nodes));
			a.join();
			b.join();
		}

		for (size_t i = 0; i < rec1Nodes.size(); ++i)
		{
			Wire(rec1Nodes[i], rec2Nodes[i], elbow).Draw(color);
		}
	}
}

void Tab::UpdateCamera()
{
	if (GetMouseWheelMove() > 0 && camera.zoom < 2.0f)
		camera.zoom *= 2;

	else if (GetMouseWheelMove() < 0 && camera.zoom > 0.125f)
		camera.zoom /= 2;

	camera.target.x += (float)(((int)IsKeyDown(KEY_RIGHT) - (int)IsKeyDown(KEY_LEFT)) * g_gridSize);
	camera.target.y += (float)(((int)IsKeyDown(KEY_DOWN) - (int)IsKeyDown(KEY_UP)) * g_gridSize);
}
