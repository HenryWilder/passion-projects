#pragma once
#include <vector>
#include <queue>
#include "raylib.h"
#include "Rect.h"

class Panel;
class Frame;

// A sub-window framel & panel for a Frame. Can be dragged and resized within the main window and displays contents.
class Panel
{
public:
	// Panels are in order of depth; first will always be focused
	static std::vector<Panel*> allPanels;
	static void SetActivePanel(size_t index);
	// Prefer MakeActivePanel(size_t) where index is already available.
	static void SetActivePanel(Panel* panel);

	static std::queue<Panel*> panelsToAdd;
	static void AddPanelAfterTick(Panel* panel);

	static std::queue<Panel*> panelsToRemove;
	static void RemovePanelAfterTick(Panel* panel);

private:
	Rect rect;
	Rect decorationRect; // Set whenever rect changes
	static constexpr RectOffset border = RectOffset(4,4,4,4); // Offset from rect to the frame
	static constexpr int decorationHeight = 17;
	static constexpr float tabWidth = 100;
	static constexpr Vector2 minSize = {
		tabWidth         + border.GetHorizontal(), // Todo: Make min width the size of the x button
		decorationHeight + border.GetVertical()
	};
	size_t tabIndex = 0; // The currently visible tab
	std::vector<Frame*> tabs;
	bool active = false;
	bool beingDragged = false;
	bool draggingTab = false;
	int newTabIndex = -1; // Index of tab that is going to be added - negative for none
	static constexpr int newTabInserterLineWidth = 5;
	Vector2 tabDelta = { 0,0 };
	enum class Axis { negative = -1, null = 0, positive = 1 };
	Axis beingResized_horizontal = Axis::null;
	Axis beingResized_vertical   = Axis::null;

	void UpdateDecorationRect();

	void MoveViewport(Vector2 delta);

public:
	static Shader ghostShader;
	static Shader gripShader;
	static int gripShaderSizeLoc;

	Rect GetContentRect() const;
	Frame* GetCurrentTab();
	const Frame* GetCurrentTab() const;

	// Checks if the point is on the panel rect
	bool PanelContains(Vector2 point);
	// Checks if the point is on the content rect
	bool ContentContains(Vector2 point);
	// Checks if the point is on panel rect but not the content
	bool BorderContains(Vector2 point);

	Panel() = default;
	Panel(Rect rect);
	Panel(Rect rect, Frame* tab);

	void AddTab(Frame* tab);
	void InsertTab(Frame* tab, size_t at);
	Frame* RemoveTab(size_t at);

	void UpdateDragAndResize();

	void TickActive();

	void TickPassive();

	// Draws decoration and content
	void Draw() const;
};
