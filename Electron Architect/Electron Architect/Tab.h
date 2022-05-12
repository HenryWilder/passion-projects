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

	inline bool SelectionExists() const
	{
		return !selection.empty();
	}
	inline size_t SelectionSize() const
	{
		return selection.size();
	}

	IRect* GetLastSelectionRec();
	const IRect* GetLastSelectionRecConst() const;
	void CreateSelectionRec(IRect rec);
	inline bool SelectionRectExists() const
	{
		return !selectionRecs.empty();
	}
	inline size_t SelectionRectCount() const
	{
		return selectionRecs.size();
	}
	bool IsSelectionBridgeable() const;
	void BridgeSelection(ElbowConfig elbow);
	// Todo: pretty expensive, having to sort the entire thing every frame! Do something!!
	void DrawBridgePreview(ElbowConfig elbow, Color color) const;

	void UpdateCamera();
};
