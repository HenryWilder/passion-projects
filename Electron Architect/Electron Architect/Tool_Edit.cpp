#include <raylib.h>
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"
#include "Blueprint.h"
#include "Group.h"
#include "NodeWorld.h"
#include "ProgramData.h"
#include "ToolModes.h"

Tool_Edit::Tool_Edit()
{
    // Start of mode
}
Tool_Edit::~Tool_Edit()
{
    // End of mode
}

void Tool_Edit::MakeGroupFromSelection()
{
    NodeWorld::Get().CreateGroup(selectionRec);
    selectionRec = IRect(0);
    data.selection.clear();
}

bool Tool_Edit::IsSelectionRectValid() const
{
    return !selectionWIP && !(selectionRec.w == 0 || selectionRec.h == 0);
}

void Tool_Edit::ClearSelection()
{
    data.selection.clear();
    selectionRec = IRect(0);
}

void Tool_Edit::TryGrouping()
{
    if (IsSelectionRectValid())
        MakeGroupFromSelection();
}

void Tool_Edit::Update()
{
    if (data.b_cursorMoved)
    {
        if (!nodeBeingDragged &&
            !wireBeingDragged &&
            !draggingGroup)
        {
            hoveredGroup = nullptr;
            data.hoveredWire = nullptr;
            data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
            if (!data.hoveredNode)
            {
                data.hoveredWire = NodeWorld::Get().FindWireElbowAtPos(data.cursorPos);
                if (!data.hoveredWire)
                {
                    hoveredGroup = NodeWorld::Get().FindGroupAtPos(data.cursorPos);
                }
            }
        }
        else if (nodeBeingDragged)
        {
            hoveringMergable = NodeWorld::Get().FindNodeAtPos(data.cursorPos); // This will come before updating the position of the dragged node
        }
    }

    // Press
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!data.selection.empty() && !!data.hoveredNode) // There is a selection, and a node has been pressed
        {
            nodeBeingDragged = data.hoveredNode;
            wireBeingDragged = nullptr;
            selectionRec = data.GetSelectionBounds();
        }
        else
        {
            data.selection.clear();
            nodeBeingDragged = data.hoveredNode;
            wireBeingDragged = data.hoveredWire;

            // selectionStart being used as an offset here
            if (draggingGroup = !!hoveredGroup)
            {
                NodeWorld::Get().FindNodesInGroup(data.selection, hoveredGroup);
                selectionStart = (data.cursorPos - (fallbackPos = hoveredGroup->GetPosition()));
            }

            fallbackPos = data.cursorPos;
            if (selectionWIP = !(nodeBeingDragged || wireBeingDragged || draggingGroup))
                selectionStart = data.cursorPos;
        }
    }

    // Selection
    if (selectionWIP)
    {
        selectionRec = IRect(
            std::minmax(data.cursorPos.x, selectionStart.x),
            std::minmax(data.cursorPos.y, selectionStart.y));
        selectionRec.DeAbuse();
    }
    // Node
    else if (!!nodeBeingDragged)
    {
        // Multiple selection
        if (!data.selection.empty())
        {
            const IVec2 offset = data.GetCursorDelta();
            for (Node* node : data.selection)
            {
                node->SetPosition_Temporary(node->GetPosition() + offset);
            }
            selectionRec.position += offset;
        }
        else
            nodeBeingDragged->SetPosition_Temporary(data.cursorPos);
    }
    // Wire
    else if (!!wireBeingDragged)
    {
        wireBeingDragged->SnapElbowToLegal(data.cursorPos);
    }
    // Group
    else if (draggingGroup)
    {
        hoveredGroup->SetPosition(data.cursorPos - selectionStart);
        for (Node* node : data.selection)
        {
            const IVec2 offset = data.GetCursorDelta();
            node->SetPosition_Temporary(node->GetPosition() + offset);
        }
    }

    // Release
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)))
    {
        // Cancel
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            if (!!nodeBeingDragged)
            {
                nodeBeingDragged->SetPosition(fallbackPos);
            }
            else if (draggingGroup)
            {
                hoveredGroup->SetPosition(fallbackPos);
                for (Node* node : data.selection)
                {
                    IVec2 offset = (fallbackPos + selectionStart) - data.cursorPos;
                    node->SetPosition_Temporary(node->GetPosition() + offset);
                }
            }
            else if (selectionWIP)
            {
                selectionRec = IRect(0);
            }
        }
        // Finalize
        else
        {
            if (!!nodeBeingDragged)
            {
                _ASSERT_EXPR(hoveringMergable != nodeBeingDragged, L"Node being dragged is trying to merge with itself");
                if (!!hoveringMergable)
                {
                    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                    {
                        data.hoveredNode = NodeWorld::Get().MergeNodes(hoveringMergable, nodeBeingDragged);
                        hoveringMergable = nodeBeingDragged = nullptr;
                    }
                    else
                    {
                        NodeWorld::Get().SwapNodes(hoveringMergable, nodeBeingDragged);
                        nodeBeingDragged->SetPosition(fallbackPos);
                    }
                }
                else
                    nodeBeingDragged->SetPosition(data.cursorPos);
            }
            else if (draggingGroup)
            {
                for (Node* node : data.selection)
                {
                    node->SetPosition(node->GetPosition());
                }
            }
            else if (selectionWIP)
            {
                NodeWorld::Get().FindNodesInRect(data.selection, selectionRec);
            }
        }
        if (draggingGroup)
            ClearSelection();
        nodeBeingDragged = nullptr;
        selectionWIP = false;
        draggingGroup = false;
        wireBeingDragged = nullptr;
    }
    // Right click
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !!data.hoveredNode)
    {
        data.hoveredNode->SetGate(data.gatePick);
        switch (data.hoveredNode->GetGate())
        {
        case Gate::RESISTOR:  data.hoveredNode->SetResistance(data.storedExtraParam); break;
        case Gate::LED:       data.hoveredNode->SetColorIndex(data.storedExtraParam); break;
        case Gate::CAPACITOR: data.hoveredNode->SetCapacity(data.storedExtraParam);   break;
        }
    }
}
void Tool_Edit::Draw()
{
    if (!!hoveredGroup)
        hoveredGroup->Highlight(INTERFERENCEGRAY);

    DrawRectangleIRect(selectionRec, ColorAlpha(SPACEGRAY, 0.5));
    DrawRectangleLines(selectionRec.x, selectionRec.y, selectionRec.w, selectionRec.h, LIFELESSNEBULA);

    NodeWorld::Get().DrawWires();

    for (Node* node : data.selection)
    {
        DrawCircleIV(node->GetPosition(), node->g_nodeRadius + 3, WIPBLUE);
    }

    if (!!data.hoveredWire)
    {
        IVec2 pts[4];
        data.hoveredWire->GetLegalElbowPositions(pts);
        for (const IVec2& p : pts)
        {
            Wire::Draw(data.hoveredWire->GetStartPos(), p, data.hoveredWire->GetEndPos(), ColorAlpha(WIPBLUE, 0.25f));
            DrawCircle(p.x, p.y, Wire::g_elbowRadius, ColorAlpha(WIPBLUE, 0.5f));
        }

        data.hoveredWire->Draw(WIPBLUE);
        Color elbowColor;
        if (!!wireBeingDragged)
            elbowColor = WIPBLUE;
        else
            elbowColor = CAUTIONYELLOW;
        data.hoveredWire->DrawElbow(elbowColor);
    }

    NodeWorld::Get().DrawNodes();

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(CAUTIONYELLOW);
    }
    if (!!data.hoveredWire)
    {
        data.hoveredWire->start->Draw(INPUTLAVENDER);
        data.hoveredWire->end->Draw(OUTPUTAPRICOT);
    }

    if (!!nodeBeingDragged && hoveringMergable)
    {
        DrawCircleIV(nodeBeingDragged->GetPosition(), Node::g_nodeRadius * 2.0f, VIOLET);
        data.DrawTooltipAtCursor(
            "Hold [shift] to merge on release.\n"
            "Otherwise, nodes will only be swapped.", VIOLET);
    }
}
