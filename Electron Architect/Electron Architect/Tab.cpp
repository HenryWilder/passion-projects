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
	
	// Must be an even number of nodes
	if (selection.size() & 1)
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
	return rec1Nodes == rec2Nodes;
}
void Tab::BridgeSelection()
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

	if (rec1Nodes.size() != rec2Nodes.size())
	{
		owningWindow->Log(LogType::warning, "Tried to bridge selection illegally");
		return;
	}

	{
		auto sortNodes = [](std::vector<Node*>& nodeVec) {
			std::sort(nodeVec.begin(), nodeVec.end(),
				[](Node* a, Node* b)
				{
					return (a->GetX() != b->GetX() ? a->GetX() < b->GetX() : a->GetY() < b->GetY());
				});
		};
		std::thread a(sortNodes, std::ref(rec1Nodes));
		std::thread b(sortNodes, std::ref(rec2Nodes));
		a.join();
		b.join();
	}

	for (size_t i = 0; i < rec1Nodes.size(); ++i)
	{
		graph->CreateWire(rec1Nodes[i], rec2Nodes[i]);
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
