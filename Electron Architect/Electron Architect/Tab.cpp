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

void Tab::UpdateBridgeCache()
{
	bridgeCache[0].clear();
	bridgeCache[1].clear();

	// Must be exactly two rectangles
	if (selectionRecs.size() != 2)
		return;

	owningWindow->Log(LogType::attempt, "Updating bridge cache");

	size_t cacheSizes[2] = { 0,0 };
	// Rule checks and cache size counting
	{
		size_t halfSelectionSize = selection.size() / 2;
		for (Node* node : selection)
		{
			if (InBoundingBox(selectionRecs[0], node->GetPosition()))
				cacheSizes[0]++;
			else
			{
				// Expected behavior check
				if (!InBoundingBox(selectionRecs[1], node->GetPosition())) [[unlikely]]
				{
					owningWindow->Log(LogType::warning, "Node in selection was in neither selection rectangle");
					return;
				}

				cacheSizes[1]++;

				// Confirmed neither is a single node, and one of them has more than half of the nodes
				// (optimization)
				if (cacheSizes[0] > 1 &&
					cacheSizes[1] > 1 &&
					(cacheSizes[0] > halfSelectionSize ||
						cacheSizes[1] > halfSelectionSize))
					[[unlikely]]
				{
					owningWindow->Log(LogType::success, "Early exit");
					return;
				}
			}
		}

		if ((selection.size() & 1) ?
			(cacheSizes[0] != 1 && cacheSizes[1] != 1) :
			(cacheSizes[0] != cacheSizes[1]))
		{
			owningWindow->Log(LogType::success, "No bridge can be made");
			return;
		}
	}

	// Produce cache
	{
		bridgeCache[0].reserve(cacheSizes[0]);
		bridgeCache[1].reserve(cacheSizes[1]);
		for (Node* node : selection)
		{
			if (InBoundingBox(selectionRecs[0], node->GetPosition()))
				bridgeCache[0].push_back(node);
			else
				bridgeCache[1].push_back(node);
		}

		auto sortNodes = [](std::vector<Node*>& nodeVec) {
			std::sort(nodeVec.begin(), nodeVec.end(),
				[](Node* a, Node* b)
				{
					return (a->GetX() != b->GetX() ? a->GetX() < b->GetX() : a->GetY() < b->GetY());
				});
		};

		std::thread a(sortNodes, std::ref(bridgeCache[0]));
		std::thread b(sortNodes, std::ref(bridgeCache[1]));
		a.join();
		b.join();
	}
	owningWindow->Log(LogType::success, "Bridge cache updated and valid");
}

void Tab::BridgeSelection(ElbowConfig elbow)
{
	_ASSERT_EXPR(IsSelectionBridgeable(), L"Selection is not bridgable");

	size_t i = 0;
	size_t i_increment = (size_t)(bridgeCache[0].size() > 1);
	size_t j = 0;
	size_t j_increment = (size_t)(bridgeCache[1].size() > 1);
	while (i < bridgeCache[0].size() || j < bridgeCache[1].size())
	{
		graph->CreateWire(bridgeCache[0][i], bridgeCache[1][j], elbow);
		i += i_increment;
		j += j_increment;
	}
	bridgeCache[0].clear();
	bridgeCache[1].clear();
}

void Tab::DrawBridgePreview(ElbowConfig elbow, Color color) const
{
	_ASSERT_EXPR(IsSelectionBridgeable(), L"Selection is not bridgable");

	size_t i = 0;
	size_t i_increment = (size_t)(bridgeCache[0].size() > 1);
	size_t j = 0;
	size_t j_increment = (size_t)(bridgeCache[1].size() > 1);
	while (i < bridgeCache[0].size() || j < bridgeCache[1].size())
	{
		Wire(bridgeCache[0][i], bridgeCache[1][j], elbow).Draw(color);
		i += i_increment;
		j += j_increment;
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
