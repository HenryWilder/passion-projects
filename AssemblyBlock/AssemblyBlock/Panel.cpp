#pragma once
#include <vector>
#include <deque>
#include <queue>
#include "rayex.h"
#include "Rect.h"
#include "Window.h"
#include "Frame.h"
#include "Panel.h"

// A sub-window framel & panel for a Frame. Can be dragged and resized within the main window and displays contents.
#pragma region Panel
// Panels are in order of depth; first will always be focused
std::deque<Panel*> Panel::allPanels;
void Panel::SetActivePanel(size_t index)
{
	if (index != 0)
	{
		Panel* panel = allPanels[index];
		allPanels.erase(allPanels.begin() + index);
		allPanels.push_front(panel);
	}
}
// Prefer MakeActivePanel(size_t) where index is already available.
void Panel::SetActivePanel(Panel* panel)
{
	auto it = std::find(allPanels.begin(), allPanels.end(), panel);
	_ASSERT(it != allPanels.end());
	SetActivePanel(it - allPanels.begin());
}

std::queue<Panel*> Panel::panelsToAdd;
void Panel::AddPanelAfterTick(Panel* panel)
{
	panelsToAdd.push(panel);
}

std::queue<Panel*> Panel::panelsToRemove;
void Panel::RemovePanelAfterTick(Panel* panel)
{
	panelsToRemove.push(panel);
}

void Panel::UpdateDecorationRect()
{
	decorationRect = border.Add(rect);
	decorationRect.Height = decorationHeight;
}

Shader Panel::ghostShader;
Shader Panel::gripShader;
int Panel::gripShaderSizeLoc;

Rect Panel::GetContentRect() const
{
	Rect contentRect = border.Add(rect);
	contentRect.yMin += decorationHeight + 1;
	return contentRect;
}
Frame* Panel::GetCurrentTab() { return tabs[tabIndex]; }
const Frame* Panel::GetCurrentTab() const { return tabs[tabIndex]; }

// Checks if the point is on the panel rect
bool Panel::PanelContains(Vector2 point)
{
	return rect.Contains(point);
}
// Checks if the point is on the content rect
bool Panel::ContentContains(Vector2 point)
{
	return GetContentRect().Contains(point);
}
// Checks if the point is on panel rect but not the content
bool Panel::BorderContains(Vector2 point)
{
	return PanelContains(point) && !ContentContains(point);
}

Panel::Panel(Rect rect) : rect(rect) { UpdateDecorationRect(); }
Panel::Panel(Rect rect, Frame* tab) : rect(rect) { UpdateDecorationRect(); AddTab(tab); }

void Panel::AddTab(Frame* tab)
{
	_ASSERT(tab != nullptr);
	tab->SetPanel(this);
	tabs.push_back(tab);
}
void Panel::InsertTab(Frame* tab, size_t at)
{
	_ASSERT(at <= tabs.size());
	_ASSERT(tab != nullptr);
	tab->SetPanel(this);
	tabs.insert(tabs.begin() + at, tab);
}
Frame* Panel::RemoveTab(size_t at)
{
	_ASSERT(at < tabs.size());
	Frame* tab = tabs[at];
	tabs.erase(tabs.begin() + at);
	tabIndex = std::min(tabIndex, tabs.size() - 1);
	return tab;
}

void Panel::UpdateDragAndResize()
{
	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		// Enter a panel
		if (draggingTab)
		{
			Rect tabRect = decorationRect;
			tabRect.Width = tabWidth;
			for (Panel* panel : allPanels)
			{
				if (panel != this && tabRect.Overlaps(panel->decorationRect))
				{
					Frame* tab = GetCurrentTab();
					panel->AddTab(tab);
					panel->tabIndex = panel->tabs.size() - 1; // Todo: insertion, not just appending
					panel->newTabIndex = -1;
					if (tabs.size() > 1) // I have a family!
					{
						RemoveTab(tabIndex);
						draggingTab = false;
						active = false;
					}
					else // My work here is done.
					{
						RemovePanelAfterTick(this);
					}
					SetActivePanel(panel); // Todo: Uhhh... Could be problematic...
					break;
				}
			}
		}
		beingDragged = false;
		draggingTab = false;
		beingResized_horizontal = Axis::null;
		beingResized_vertical = Axis::null;
		return;
	}

	else if (beingDragged || draggingTab)
	{
		if (beingDragged)
		{
			Vector2 delta = GetMouseDelta();
			rect.Position += delta;
			decorationRect.Position += delta;
			MoveViewport(delta);
		}
		if (draggingTab)
		{
			tabDelta += GetMouseDelta();
			// Want to enter a panel
			Rect tabRect = decorationRect;
			tabRect.Width = tabWidth;
			for (Panel* panel : allPanels)
			{
				if (panel != this && tabRect.Overlaps(panel->decorationRect))
					panel->newTabIndex = panel->tabs.size();
				else
					panel->newTabIndex = -1;
			}
			// Exit the panel
			if (!beingDragged && !decorationRect.Contains(Window::mousePos))
			{
				Rect newPanelRect = rect;
				newPanelRect.Position += tabDelta;
				newPanelRect.X += tabIndex * tabWidth;
				Frame* tab = GetCurrentTab();
				Panel* newPanel = new Panel(newPanelRect, tab);
				newPanel->beingDragged = true;
				newPanel->draggingTab = true;
				newPanel->active = true;
				AddPanelAfterTick(newPanel);
				RemoveTab(tabIndex);
				draggingTab = false;
				active = false;
			}
		}
	}
	else
	{
		Vector2 actualChange = { 0,0 };
		switch (beingResized_horizontal)
		{
		case Panel::Axis::negative:
			actualChange.x = -rect.xMin;
			rect.xMin = std::min(Window::mousePos.x, rect.xMax - minSize.x);
			actualChange.x += rect.xMin;
			break;

		case Panel::Axis::positive:
			actualChange.x = -rect.xMax;
			rect.xMax = std::max(Window::mousePos.x, rect.xMin + minSize.x);
			actualChange.x += rect.xMax;
			break;

		default: break;
		}

		switch (beingResized_vertical)
		{
		case Panel::Axis::negative:
			actualChange.y = -rect.yMin;
			rect.yMin = std::min(Window::mousePos.y, rect.yMax - minSize.y);
			actualChange.y += rect.yMin;
			break;

		case Panel::Axis::positive:
			actualChange.y = -rect.yMax;
			rect.yMax = std::max(Window::mousePos.y, rect.yMin + minSize.y);
			actualChange.y += rect.yMax;
			break;

		default: break;
		}

		Vector2 move = actualChange * Vector2{
			abs((float)beingResized_horizontal),
			abs((float)beingResized_vertical)
		};
		MoveViewport(move * 0.5f);

		UpdateDecorationRect();
	}
}

void Panel::TickActive()
{
	active = true;
	UpdateDragAndResize();

	// On content
	if (ContentContains(Window::mousePos))
	{
		GetCurrentTab()->TickActive();
		return;
	}

	// On border/decoration
	if (PanelContains(Window::mousePos))
	{
		Axis canResize_horizontal = Axis::null;
		Axis canResize_vertical = Axis::null;

		// On border
		if (!decorationRect.Contains(Window::mousePos))
		{
			Rect contentRect = border.Add(rect);

			if (Window::mousePos.x < contentRect.xMin)
				canResize_horizontal = Axis::negative;
			else if (Window::mousePos.x > contentRect.xMax)
				canResize_horizontal = Axis::positive;

			if (Window::mousePos.y < contentRect.yMin)
				canResize_vertical = Axis::negative;
			else if (Window::mousePos.y > contentRect.yMax)
				canResize_vertical = Axis::positive;

			if ((canResize_horizontal == Axis::negative && canResize_vertical == Axis::negative) ||
				(canResize_horizontal == Axis::positive && canResize_vertical == Axis::positive))
				Window::cursor = MOUSE_CURSOR_RESIZE_NWSE;
			else if ((canResize_horizontal == Axis::negative && canResize_vertical == Axis::positive) ||
				(canResize_horizontal == Axis::positive && canResize_vertical == Axis::negative))
				Window::cursor = MOUSE_CURSOR_RESIZE_NESW;
			else if (canResize_horizontal != Axis::null)
				Window::cursor = MOUSE_CURSOR_RESIZE_EW;
			else if (canResize_vertical != Axis::null)
				Window::cursor = MOUSE_CURSOR_RESIZE_NS;
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			// On decoration
			if (decorationRect.Contains(Window::mousePos))
			{
				unsigned clickIndex = (unsigned)((Window::mousePos.x - decorationRect.xMin) / tabWidth);
				if (clickIndex < tabs.size())
				{
					tabIndex = clickIndex;
					if (tabs.size() == 1)
						beingDragged = true;
					draggingTab = true;
					tabDelta = { 0,0 };
				}
				else
					beingDragged = true;
			}
			else if (!decorationRect.Contains(Window::mousePos))
			{
				beingResized_horizontal = canResize_horizontal;
				beingResized_vertical   = canResize_vertical;
			}
		}
	}
	if ((beingResized_horizontal == Axis::negative && beingResized_vertical == Axis::negative) ||
		(beingResized_horizontal == Axis::positive && beingResized_vertical == Axis::positive))
		Window::cursor =MOUSE_CURSOR_RESIZE_NWSE;
	else if ((beingResized_horizontal == Axis::negative && beingResized_vertical == Axis::positive) ||
		(beingResized_horizontal == Axis::positive && beingResized_vertical == Axis::negative))
		Window::cursor = MOUSE_CURSOR_RESIZE_NESW;
	else if (beingResized_horizontal != Axis::null)
		Window::cursor = MOUSE_CURSOR_RESIZE_EW;
	else if (beingResized_vertical != Axis::null)
		Window::cursor = MOUSE_CURSOR_RESIZE_NS;

	GetCurrentTab()->TickPassive();
}

void Panel::TickPassive()
{
	active = false;
	UpdateDragAndResize();

	if (PanelContains(Window::mousePos))
	{
		bool mouseIsBlocked = false;
		for (Panel* panel : allPanels)
		{
			if (panel == this)
				continue;

			if (panel->PanelContains(Window::mousePos))
			{
				mouseIsBlocked = true;
				break;
			}
		}
		// On border
		if (!mouseIsBlocked && BorderContains(Window::mousePos))
		{
			Axis canResize_horizontal = Axis::null;
			Axis canResize_vertical = Axis::null;
			Rect contentRect = border.Add(rect);

			if (Window::mousePos.x < contentRect.xMin)
				canResize_horizontal = Axis::negative;
			else if (Window::mousePos.x > contentRect.xMax)
				canResize_horizontal = Axis::positive;

			if (Window::mousePos.y < contentRect.yMin)
				canResize_vertical = Axis::negative;
			else if (Window::mousePos.y > contentRect.yMax)
				canResize_vertical = Axis::positive;

			if ((canResize_horizontal == Axis::negative && canResize_vertical == Axis::negative) ||
				(canResize_horizontal == Axis::positive && canResize_vertical == Axis::positive))
				Window::cursor = MOUSE_CURSOR_RESIZE_NWSE;
			else if ((canResize_horizontal == Axis::negative && canResize_vertical == Axis::positive) ||
				(canResize_horizontal == Axis::positive && canResize_vertical == Axis::negative))
				Window::cursor = MOUSE_CURSOR_RESIZE_NESW;
			else if (canResize_horizontal != Axis::null)
				Window::cursor = MOUSE_CURSOR_RESIZE_EW;
			else if (canResize_vertical != Axis::null)
				Window::cursor = MOUSE_CURSOR_RESIZE_NS;
		}
	}

	GetCurrentTab()->TickPassive();
}

// Draws decoration and content
void Panel::Draw() const
{
	rect.Draw({ 60, 60, 60, 255 });
	RectOffset::expand.Add(border.Add(rect)).Draw({ 100,100,100,255 });

	// Decoration
	decorationRect.Draw({ 60, 60, 60, 255 });
	Rect tabRect = decorationRect;
	tabRect.Width = tabWidth;
	constexpr int fontSize = 10;
	int textY = rect.yMin + border.top + (border.top + decorationHeight - fontSize) / 2;
	bool breakAfterTab = false;
	for (size_t i = 0; i < tabs.size() && !breakAfterTab; ++i)
	{
		if (tabRect.xMax > decorationRect.xMax)
		{
			tabRect.xMax = decorationRect.xMax;
			breakAfterTab = true;
		}
		if (i == tabIndex) tabRect.Draw(active ? BLUE : Color{ 100,100,100,255 });
		DrawLineV({ tabRect.xMax - 0.5f, tabRect.yMin }, { tabRect.xMax - 0.5f, tabRect.yMax }, { 100,100,100,255 });
		BeginScissorMode(tabRect);
		DrawText(tabs[i]->GetName(), tabRect.xMin + border.left, textY, fontSize, RAYWHITE);
		EndScissorMode();
		tabRect.X += tabWidth;
	}
	if (newTabIndex >= 0)
	{
		Rect inserterRect = {};
		inserterRect.yMin = decorationRect.yMin;
		inserterRect.yMax = decorationRect.yMax;
		inserterRect.xMin = decorationRect.xMin + tabWidth * newTabIndex;
		inserterRect.Width = newTabInserterLineWidth;
		inserterRect.Draw({ 0, 127, 255, 255 });
	}
	Rect gripRect = tabRect;
	gripRect.xMax = decorationRect.xMax;
	gripRect = RectOffset(5,5,5,5).Add(gripRect);
	if (gripRect.Width > 0.0f)
	{
		float size[2] = { gripRect.Width, gripRect.Height };
		SetShaderValue(gripShader, gripShaderSizeLoc, size, SHADER_UNIFORM_VEC2);
		BeginShaderMode(gripShader);
		DrawTexturePro(texShapes, texShapesRec, (Rectangle)gripRect, { 0,0 }, 0.0f, { 100,100,100,255 });
		EndShaderMode();
	}

	// Frame
	Rect contentRect = GetContentRect();
	BeginScissorMode(contentRect);
	ClearBackground({ 40,40,40,255 });
	GetCurrentTab()->Draw();
	EndScissorMode();
}

#pragma endregion

#include "Viewport.h"

void Panel::MoveViewport(Vector2 delta)
{
	for (Frame* frame : tabs)
	{
		if (Viewport* viewport = dynamic_cast<Viewport*>(frame))
			viewport->MoveViewport(delta);
	}
}
