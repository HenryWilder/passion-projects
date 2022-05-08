#pragma once

struct IRect;
class Node;
class Graph;
struct Window;

struct Tab
{
	Tab(Window* owner, const char* name = "Unnamed graph");
	~Tab();

	Window* owningWindow;

	Camera2D camera;
	Graph* graph;
	std::vector<Node*> selection;
	std::vector<IRect> selectionRecs;

	bool SelectionExists() const;
	size_t SelectionSize() const;

	IRect* GetLastSelectionRec();
	const IRect* GetLastSelectionRecConst() const;
	void CreateSelectionRec(IRect rec);
	bool SelectionRectExists() const;
	size_t SelectionRectCount() const;

	void UpdateCamera();
};
