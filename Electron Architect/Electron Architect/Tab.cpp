#include <raylib.h>
#include "IVec.h"
#include "Graph.h"
#include "Tab.h"

Tab::Tab(const char* name) :
	camera{ .offset{ 0,0 }, .target{ 0,0 }, .rotation{ 0 }, .zoom{ 1 } },
	graph(new Graph(name)) {}

Tab::~Tab()
{
	delete graph;
}

bool Tab::SelectionExists() const
{
	return !selection.empty();
}

size_t Tab::SelectionSize() const
{
	return selection.size();
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

bool Tab::SelectionRectExists() const
{
	return !selectionRecs.empty();
}

size_t Tab::SelectionRectCount() const
{
	return selectionRecs.size();
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
