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

		if (!(cacheSizes[0] > 0 && cacheSizes[1] > 0) ||
		   (!(cacheSizes[0] == 1 && cacheSizes[1] > 1) &&
			!(cacheSizes[1] == 1 && cacheSizes[0] > 1) &&
			!(cacheSizes[0] == cacheSizes[1])))
		{
			owningWindow->Log(LogType::success, "No bridge can be made");
			owningWindow->Log(LogType::info, "Selection 1 size: " + std::to_string(cacheSizes[0]));
			owningWindow->Log(LogType::info, "Selection 2 size: " + std::to_string(cacheSizes[1]));
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

	if (bridgeCache[0].size() == bridgeCache[1].size())
	{
		for (size_t i = 0; i < bridgeCache[0].size(); ++i)
		{
			graph->CreateWire(bridgeCache[0][i], bridgeCache[1][i], elbow);
		}
	}
	else if (bridgeCache[0].size() == 1)
	{
		for (size_t i = 0; i < bridgeCache[1].size(); ++i)
		{
			graph->CreateWire(bridgeCache[0][0], bridgeCache[1][i], elbow);
		}
	}
	else if (bridgeCache[1].size() == 1)
	{
		for (size_t i = 0; i < bridgeCache[0].size(); ++i)
		{
			graph->CreateWire(bridgeCache[0][i], bridgeCache[1][0], elbow);
		}
	}
	bridgeCache[0].clear();
	bridgeCache[1].clear();
}

void Tab::DrawBridgePreview(ElbowConfig elbow, Color color) const
{
	_ASSERT_EXPR(IsSelectionBridgeable(), L"Selection is not bridgable");

	if (bridgeCache[0].size() == bridgeCache[1].size())
	{
		for (size_t i = 0; i < bridgeCache[0].size(); ++i)
		{
			Wire(bridgeCache[0][i], bridgeCache[1][i], elbow).Draw(color);
		}
	}
	else if (bridgeCache[0].size() == 1)
	{
		for (size_t i = 0; i < bridgeCache[1].size(); ++i)
		{
			Wire(bridgeCache[0][0], bridgeCache[1][i], elbow).Draw(color);
			graph->CreateWire(bridgeCache[0][0], bridgeCache[1][i], elbow);
		}
	}
	else if (bridgeCache[1].size() == 1)
	{
		for (size_t i = 0; i < bridgeCache[0].size(); ++i)
		{
			Wire(bridgeCache[0][i], bridgeCache[1][0], elbow).Draw(color);
		}
	}
}

void Tab::Set2DMode(bool value)
{
	static bool in2DMode = false;
	if (in2DMode != value)
	{
		in2DMode = value;
		if (value)
			BeginMode2D(camera);
		else
			EndMode2D();
	}
}

void Tab::UpdateCamera()
{
	if (GetMouseWheelMove() > 0 && camera.zoom < 2.0f)
		camera.zoom *= 2;

	else if (GetMouseWheelMove() < 0 && camera.zoom > 0.125f)
		camera.zoom /= 2;

	if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
	{
		Vector2 delta = GetMouseDelta();
		delta.x = (float)((int)(delta.x / ((float)g_gridSize * camera.zoom)) * g_gridSize);
		delta.y = (float)((int)(delta.y / ((float)g_gridSize * camera.zoom)) * g_gridSize);
		camera.target = camera.target + delta;
	}
	else
	{
		camera.target.x += (float)(((int)IsKeyDown(KEY_RIGHT) - (int)IsKeyDown(KEY_LEFT)) * g_gridSize) / std::min(camera.zoom, 1.0f);
		camera.target.y += (float)(((int)IsKeyDown(KEY_DOWN)  - (int)IsKeyDown(KEY_UP))   * g_gridSize) / std::min(camera.zoom, 1.0f);
	}
}
