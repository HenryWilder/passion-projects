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
	bridgeCache.clear();

	// Must be at two rectangles
	if (selectionRecs.size() < 2)
		return;

	owningWindow->Log(LogType::attempt, "Updating bridge cache");

	std::vector<size_t> cacheSizes;
	cacheSizes.resize(selectionRecs.size(), 0);
	// Rule checks and cache size counting
	{
		size_t halfSelectionSize = selection.size() / 2;
		for (Node* node : selection)
		{
			for (size_t i = 0; i < selectionRecs.size(); ++i)
			{
				if (InBoundingBox(selectionRecs[i], node->GetPosition()))
				{
					++cacheSizes[i];
					break;
				}
			}
		}

		if (!(cacheSizes[0] == 1 || cacheSizes.back() == 1))
		{
			size_t size1 = cacheSizes[0];
			bool invalid = false;
			for (size_t size : cacheSizes)
			{
				if (size != size1)
				{
					invalid = true;
					break;
				}
			}
			if (invalid)
			{
				owningWindow->Log(LogType::success, "No bridge can be made.");
				for (size_t i = 0; i < cacheSizes.size(); ++i)
				{
					owningWindow->Log(LogType::info, "Selection " + std::to_string(i) + " size = " + std::to_string(cacheSizes[i]));
				}
				return;
			}
		}
	}

	// Produce cache
	{
		bridgeCache.reserve(cacheSizes.size());
		for (size_t i = 0; i < cacheSizes.size(); ++i)
		{
			bridgeCache.push_back({});
			bridgeCache[i].reserve(cacheSizes[i]);
		}
		for (Node* node : selection)
		{
			for (size_t i = 0; i < selectionRecs.size(); ++i)
			{
				if (InBoundingBox(selectionRecs[i], node->GetPosition()))
				{
					bridgeCache[i].push_back(node);
					break;
				}
			}
		}

		auto sortNodes = [](std::vector<Node*>& nodeVec) {
			std::sort(nodeVec.begin(), nodeVec.end(),
				[](Node* a, Node* b)
				{
					return (a->GetX() != b->GetX() ? a->GetX() < b->GetX() : a->GetY() < b->GetY());
				});
		};

		std::vector<std::thread> sorterThreads;
		sorterThreads.reserve(bridgeCache.size());
		for (size_t i = 0; i < bridgeCache.size(); ++i)
		{
			sorterThreads.emplace_back(sortNodes, std::ref(bridgeCache[i]));
		}
		for (size_t i = 0; i < bridgeCache.size(); ++i)
		{
			sorterThreads[i].join();
		}
		sorterThreads.clear();
	}
	owningWindow->Log(LogType::success, "Bridge cache updated and valid");
}

void Tab::BridgeSelection(ElbowConfig elbow)
{
	_ASSERT_EXPR(IsSelectionBridgeable(), L"Selection is not bridgable");

	if (bridgeCache[0].size() == 1)
	{
		for (size_t i = 1; i < bridgeCache.size(); ++i)
		{
			for (size_t j = 0; j < bridgeCache[i].size(); ++j)
			{
				graph->CreateWire(bridgeCache[0].back(), bridgeCache[i][j], elbow);
			}
		}
	}
	else if (bridgeCache.back().size() == 1)
	{
		for (size_t i = 0; i < bridgeCache.size() - 1; ++i)
		{
			for (size_t j = 0; j < bridgeCache[i].size(); ++j)
			{
				graph->CreateWire(bridgeCache[i][j], bridgeCache.back().back(), elbow);
			}
		}
	}
	else // Already know it passed the "IsSelectionBridgeable()" checks
	{
		for (size_t i = 1; i < bridgeCache[0].size() - 1; ++i)
		{
			for (size_t j = 0; j < bridgeCache[i].size(); ++j)
			{
				graph->CreateWire(bridgeCache[i - 1][j], bridgeCache[i][j], elbow);
			}
		}
	}
	bridgeCache.clear();
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
	camera.offset = owningWindow->WindowExtents() / 2;

	if (GetMouseWheelMove() > 0 && camera.zoom < 4.0f)
		camera.zoom *= 2;

	else if (GetMouseWheelMove() < 0 && camera.zoom > 0.125f)
		camera.zoom /= 2;

	if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
	{
		Vector2 delta = GetMouseDelta();
		delta.x = (float)(roundf(delta.x / ((float)g_gridSize * camera.zoom)) * g_gridSize);
		delta.y = (float)(roundf(delta.y / ((float)g_gridSize * camera.zoom)) * g_gridSize);
		camera.target = camera.target - delta;
	}
	else
	{
		camera.target.x += (float)(((int)IsKeyDown(KEY_RIGHT) - (int)IsKeyDown(KEY_LEFT)) * g_gridSize) / std::min(camera.zoom, 1.0f);
		camera.target.y += (float)(((int)IsKeyDown(KEY_DOWN)  - (int)IsKeyDown(KEY_UP))   * g_gridSize) / std::min(camera.zoom, 1.0f);
	}
}
