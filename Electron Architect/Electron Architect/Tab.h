#pragma once

struct IRect;
class Node;
class Graph;
struct Window;

enum class WireBridgeType
{
	none, // No bridge possible (uneven selection counts + the first & last selections contain multiple nodes)
	one_to_many, // One node as input to many output nodes
	many_to_one, // Many nodes as input to one output node
	even, // All selections in the bridge contain equal nodes
};

struct Tab
{
	Tab(Window* owner, const char* name = "Unnamed graph");
	~Tab();

	Window* owningWindow;

	Camera2D camera;
	Graph* graph;
	std::vector<Node*> selection;
private:
	// Note: Make sure not to modify this without updating the bridge cache!
	std::vector<IRect> selectionRecs;
public:
	std::vector<std::vector<Node*>> bridgeCache;
	WireBridgeType cachedBridgeType;

	inline bool SelectionExists() const
	{
		return !selection.empty();
	}
	inline size_t SelectionSize() const
	{
		return selection.size();
	}

	inline bool SelectionRectExists() const
	{
		return !selectionRecs.empty();
	}
	inline size_t SelectionRectCount() const
	{
		return selectionRecs.size();
	}

	inline const std::vector<IRect>& SelectionRecs() const
	{
		return selectionRecs;
	}
	inline IRect* GetLastSelectionRec()
	{
		return SelectionRectExists() ? &selectionRecs.back() : nullptr;
	}
	inline const IRect* GetLastSelectionRecConst() const
	{
		return SelectionRectExists() ? &selectionRecs.back() : nullptr;
	}

	inline void AddSelectionRec(IRect rec)
	{
		selectionRecs.push_back(rec);
	}
	inline void ConfirmLastSelectionRec()
	{
		if (!selectionRecs.empty() &&
			(selectionRecs.back().w <= g_gridSize ||
			 selectionRecs.back().h <= g_gridSize))
		{
			selectionRecs.pop_back();
		}
	}
	inline void PopSelectionRec()
	{
		selectionRecs.pop_back();
		UpdateBridgeCache();
	}
	inline void ClearSelection()
	{
		selection.clear();
		selectionRecs.clear();
		bridgeCache.clear();
		cachedBridgeType = WireBridgeType::none;
	}

	void UpdateBridgeCache();
	inline bool IsSelectionBridgeable() const
	{
		return !bridgeCache.empty();
	}
	void BridgeSelection(ElbowConfig elbow);
	void DrawBridgePreview(ElbowConfig elbow, Color color) const;

	void UpdateCamera();

	void Set2DMode(bool value);
};
